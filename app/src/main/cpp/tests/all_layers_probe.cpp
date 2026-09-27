#include "gguf_reader.h"
#include "gguf_file.h"
#include "qwen_reference.h"
#include "transformer_layer.h"
#include <algorithm>
#include <cmath>
#include <fstream>
#include <iostream>
#include <vector>
int main(){
 const char* p="/tmp/waqti-models/qwen2.5-0.5b-instruct-q4_k_m.gguf"; std::ifstream f(p,std::ios::binary); f.seekg(0,std::ios::end); auto n=f.tellg(); f.seekg(0); std::vector<uint8_t>b((size_t)n); f.read((char*)b.data(),b.size());
 auto parsed=waqti::gguf::parse_metadata(b); std::string e; waqti::qwen::Config cfg; if(!waqti::qwen::config_from_metadata(parsed.metadata,cfg,&e)){std::cerr<<e;return 1;} waqti::gguf::MappedFile file; if(!file.open(p,&e)){std::cerr<<e;return 1;}
 waqti::gguf::TensorView emb; if(!file.view(parsed.metadata,"token_embd.weight",emb,&e)){std::cerr<<e;return 1;} std::vector<float> hidden; if(!waqti::qwen::embedding_row(emb,9707,hidden,&e)){std::cerr<<e;return 1;}
 for(size_t layer=0;layer<cfg.block_count;++layer){std::vector<float> next; waqti::transformer::LayerDiagnostics d; if(!waqti::transformer::forward_layer(file,parsed.metadata,cfg,layer,hidden,0,next,&d,&e)){std::cerr<<"FIRST_FAILURE layer="<<layer<<" error="<<e<<"\n";return 2;} std::cout<<"Layer "<<layer<<" input_rms="<<d.input_rms<<" output_rms="<<d.output_rms<<" min="<<d.minimum<<" max="<<d.maximum<<" mean="<<d.mean<<" finite="<<d.finite<<"\n"; hidden.swap(next);}
 waqti::gguf::TensorView norm,outw; if(!file.view(parsed.metadata,"output_norm.weight",norm,&e)||!file.view(parsed.metadata,"output.weight",outw,&e)){std::cerr<<e;return 3;} std::vector<float> final_hidden,logits; if(!waqti::qwen::final_norm_and_logits(hidden,norm,outw,(float)cfg.rms_epsilon,final_hidden,logits,&e)){std::cerr<<e;return 4;} size_t best=0;float bestv=0;if(!waqti::qwen::argmax(logits,best,bestv,&e)){std::cerr<<e;return 5;} std::vector<size_t> ids(logits.size());for(size_t i=0;i<ids.size();++i)ids[i]=i;std::partial_sort(ids.begin(),ids.begin()+10,ids.end(),[&](size_t a,size_t b){return logits[a]>logits[b];}); std::cout<<"FULL_MODEL_DIAGNOSTIC layers="<<cfg.block_count<<" final_rms="<<waqti::transformer::LayerDiagnostics{}.output_rms<<" logits="<<logits.size()<<" argmax="<<best<<" logit="<<bestv<<" min="<<*std::min_element(logits.begin(),logits.end())<<" max="<<*std::max_element(logits.begin(),logits.end())<<"\nTOP10";for(int i=0;i<10;++i)std::cout<<" "<<ids[i]<<":"<<logits[ids[i]];std::cout<<"\n";return 0;
}
