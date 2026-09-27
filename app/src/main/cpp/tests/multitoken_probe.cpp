#include "gguf_reader.h"
#include "gguf_file.h"
#include "qwen_reference.h"
#include "sequence_reference.h"
#include <cmath>
#include <fstream>
#include <iostream>
#include <vector>
int main(){const char*p="/tmp/waqti-models/qwen2.5-0.5b-instruct-q4_k_m.gguf";std::ifstream f(p,std::ios::binary);f.seekg(0,std::ios::end);auto n=f.tellg();f.seekg(0);std::vector<uint8_t>b((size_t)n);f.read((char*)b.data(),b.size());auto m=waqti::gguf::parse_metadata(b);std::string e;waqti::qwen::Config c;if(!waqti::qwen::config_from_metadata(m.metadata,c,&e)){std::cerr<<e;return 1;}waqti::gguf::MappedFile file;if(!file.open(p,&e)){std::cerr<<e;return 1;}for(auto ids:std::vector<std::vector<uint32_t>>{{9707},{9707,11}}){waqti::sequence::Result r;if(!waqti::sequence::forward(file,m.metadata,c,ids,r,&e)){std::cerr<<"sequence failed: "<<e;return 2;}std::cout<<"sequence_len="<<ids.size()<<" results="<<r.logits.size()<<"\n";for(size_t p0=0;p0<ids.size();++p0){size_t best=0;float val=0;if(!waqti::qwen::argmax(r.logits[p0],best,val,&e)){std::cerr<<e;return 3;}float rms=0;for(float x:r.hidden[p0]){if(!std::isfinite(x)){std::cerr<<"nonfinite\n";return 4;}rms+=x*x;}rms=std::sqrt(rms/r.hidden[p0].size());std::cout<<" position="<<p0<<" hidden_rms="<<rms<<" argmax="<<best<<" logit="<<val<<" logits="<<r.logits[p0].size()<<"\n";}}return 0;}
