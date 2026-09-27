import struct, math, sys, numpy as np
P='/tmp/waqti-models/qwen2.5-0.5b-instruct-q4_k_m.gguf'; DTYPE=np.float32

def u32(b,o): return struct.unpack_from('<I',b,o)[0],o+4
def u64(b,o): return struct.unpack_from('<Q',b,o)[0],o+8
def string(b,o):
 n,o=u64(b,o); return b[o:o+n].decode('utf-8',errors='replace'),o+n
def val(b,o,t):
 if t==0:return b[o],o+1
 if t==1:return struct.unpack_from('<b',b,o)[0],o+1
 if t==2:return struct.unpack_from('<H',b,o)[0],o+2
 if t==3:return struct.unpack_from('<h',b,o)[0],o+2
 if t==4:return u32(b,o)
 if t==5:return struct.unpack_from('<i',b,o)[0],o+4
 if t==6:return struct.unpack_from('<f',b,o)[0],o+4
 if t==7:return bool(b[o]),o+1
 if t==8:return string(b,o)
 if t==9:
  et,o=u32(b,o); n,o=u64(b,o); a=[]
  for _ in range(n): x,o=val(b,o,et);a.append(x)
  return a,o
 if t==10:return u64(b,o)
 if t==11:return struct.unpack_from('<q',b,o)[0],o+8
 if t==12:return struct.unpack_from('<d',b,o)[0],o+8
 raise ValueError(t)

def parse(path):
 b=open(path,'rb').read(); assert b[:4]==b'GGUF'; ver=struct.unpack_from('<I',b,4)[0];nt,o=u64(b,8); nk,o=u64(b,o); kv={}
 for _ in range(nk):
  k,o=string(b,o);t,o=u32(b,o);kv[k],o=val(b,o,t)
 ts=[]
 for _ in range(nt):
  name,o=string(b,o); nd,o=u32(b,o); dims=[]
  for _ in range(nd): d,o=u64(b,o);dims.append(d)
  typ,o=u32(b,o); off,o=u64(b,o);ts.append((name,dims,typ,off))
 align=int(kv.get('general.alignment',32)); dataoff=(o+align-1)//align*align
 return b,kv,{x[0]:x[1:] for x in ts},dataoff

b,kv,T,D=parse(P)
def half(x): return np.frombuffer(x,dtype='<f2').astype(np.float32)[0]
def f16(p): return half(b[p:p+2])
def decode_row(info, p):
 dims,typ,off=info; n=int(np.prod(dims)); out=np.empty(n,np.float32); q=p
 if typ==0:return np.frombuffer(b[p:p+4*n],dtype='<f4').copy()
 if typ==1:return np.frombuffer(b[p:p+2*n],dtype='<f2').astype(np.float32)
 if typ==8:
  for z in range(0,n,32):
   s=f16(q); q+=2; out[z:z+32]=s*np.frombuffer(b[q:q+32],dtype=np.int8);q+=32
  return out
 if typ==6:
  for z in range(0,n,32):
   s=f16(q); high=int.from_bytes(b[q+2:q+6],'little'); q+=6
   for i in range(32):
    low=(b[q+i//2]>>(4*(i%2)))&15; out[z+i]=s*((low|(((high>>i)&1)<<4))-16)
   q+=16
  return out
 if typ==14:
  for block in range(n//256):
   src=p+block*210; ql=src;qh=src+128;sc=src+192;s=f16(src+208)
   for nn in (0,128):
    for l in range(32):
     is_=l//16
     q1=(b[ql+l]&15)|(((b[qh+l]>>0)&3)<<4);q2=(b[ql+l+32]&15)|(((b[qh+l]>>2)&3)<<4)
     q3=((b[ql+l]>>4)&15)|(((b[qh+l]>>4)&3)<<4);q4=((b[ql+l+32]>>4)&15)|(((b[qh+l]>>6)&3)<<4)
     def si(v): return v-256 if v >= 128 else v
     out[block*256+nn+l]=s*si(b[sc+is_+0])*(q1-32);out[block*256+nn+l+32]=s*si(b[sc+is_+2])*(q2-32);out[block*256+nn+l+64]=s*si(b[sc+is_+4])*(q3-32);out[block*256+nn+l+96]=s*si(b[sc+is_+6])*(q4-32)
    ql+=64;qh+=32;sc+=8
  return out
 if typ==12:
  for block in range(n//256):
   src=p+block*144; d=f16(src);dm=f16(src+2);sc=src+4;qs=src+16;q=0
   for g in range(8):
    if g<4: scale=b[sc+g]&63; mn=b[sc+g+4]&63
    else: scale=(b[sc+g+4]&15)|((b[sc+g-4]>>6)<<4);mn=(b[sc+g+4]>>4)|((b[sc+g]>>6)<<4)
    for l in range(16): out[block*256+g*32+2*l]=d*scale*(b[qs+q]&15)-dm*mn;out[block*256+g*32+2*l+1]=d*scale*(b[qs+q]>>4)-dm*mn;q+=1
  return out
 raise ValueError((typ,dims))
def row(info,row):
 dims,typ,off=info;n=int(dims[0]);
 sizes={0:4,1:2,8:34,6:22,14:210,12:144}; block=32 if typ in (8,6) else 256 if typ in (14,12) else 1
 rb=(n//block)*sizes[typ];return decode_row(([n],typ,off),D+off+row*rb)
def matvec(info,x):
 dims=info[0];return np.array([np.sum(row(info,i)*x,dtype=np.float32) for i in range(int(dims[1]))],np.float32)
def norm(x,w):return x*(1/np.sqrt(np.mean(x*x,dtype=np.float32)+1e-6))*row(w,0)
def rope(x,heads,dim,pos):
 y=x.copy()
 for h in range(heads):
  for j in range(0,dim,2):
   a=pos*1000000.0**(-j/dim);c=math.cos(a);s=math.sin(a);u=x[h*dim+j];v=x[h*dim+j+1];y[h*dim+j]=u*c-v*s;y[h*dim+j+1]=u*s+v*c
 return y

def main():
 cfg=dict(E=int(kv['qwen2.embedding_length']),L=int(kv['qwen2.block_count']),H=int(kv['qwen2.attention.head_count']),K=int(kv['qwen2.attention.head_count_kv']),F=int(kv['qwen2.feed_forward_length']))
 emb=T['token_embd.weight'];outn=T['output_norm.weight'];outw=T['output.weight']; ids=[9707,11,11,11]; caches=[([],[]) for _ in range(cfg['L'])]; res=[]
 for pos,tok in enumerate(ids):
  x=row(emb,tok)
  for layer in range(cfg['L']):
   pre=norm(x,T[f'blk.{layer}.attn_norm.weight']);q=rope(matvec(T[f'blk.{layer}.attn_q.weight'],pre),cfg['H'],64,pos);k=rope(matvec(T[f'blk.{layer}.attn_k.weight'],pre),cfg['K'],64,pos);v=matvec(T[f'blk.{layer}.attn_v.weight'],pre);caches[layer][0].append(k);caches[layer][1].append(v);ks=np.array(caches[layer][0]);vs=np.array(caches[layer][1]);a=np.empty(cfg['H']*64,np.float32)
   for h in range(cfg['H']):
    kh=h//(cfg['H']//cfg['K']); scores=ks[:,kh*64:(kh+1)*64]@q[h*64:(h+1)*64]/math.sqrt(64); scores=np.exp(scores-scores.max());scores/=scores.sum();a[h*64:(h+1)*64]=scores@vs[:,kh*64:(kh+1)*64]
   x=x+matvec(T[f'blk.{layer}.attn_output.weight'],a);ff=norm(x,T[f'blk.{layer}.ffn_norm.weight']);x=x+matvec(T[f'blk.{layer}.ffn_down.weight'],1/(1+np.exp(-matvec(T[f'blk.{layer}.ffn_gate.weight'],ff)))*matvec(T[f'blk.{layer}.ffn_up.weight'],ff))
  fh=norm(x,outn);log=np.array([np.sum(row(outw,i)*fh,dtype=np.float32) for i in range(int(outw[0][1]))],np.float32);res.append((x.copy(),fh,log));print('python position',pos,'rms',float(np.sqrt(np.mean(x*x))),'argmax',int(log.argmax()),'logit',float(log.max()),flush=True)
 with open('/tmp/waqti_multitoken_py.bin','wb') as o:
  o.write(struct.pack('<III',len(ids),cfg['E'],int(outw[0][1])));[(o.write(x.tobytes()),o.write(y.tobytes()),o.write(z.tobytes())) for x,y,z in res]
main()
