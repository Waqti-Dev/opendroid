#include "gguf_reader.h"
#include "gguf_file.h"
#include "qwen_reference.h"
#include "sequence_reference.h"
#include <fstream>
#include <iostream>
#include <vector>
int main(){const char*p="/tmp/waqti-models/qwen2.5-0.5b-instruct-q4_k_m.gguf";std::ifstream f(p,std::ios::binary);f.seekg(0,std::ios::end);auto n=f.tellg();f.seekg(0);std::vector<uint8_t>b((size_t)n);f.read((char*)b.data(),b.size());auto m=waqti::gguf::parse_metadata(b);std::string e;waqti::qwen::Config c;if(!waqti::qwen::config_from_metadata(m.metadata,c,&e)){std::cerr<<e;return 1;}waqti::gguf::MappedFile file;if(!file.open(p,&e)){std::cerr<<e;return 1;}std::vector<uint32_t> ids{9707,11,11,11};waqti::sequence::Result r;if(!waqti::sequence::forward(file,m.metadata,c,ids,r,&e)){std::cerr<<e;return 2;}std::ofstream o("/tmp/waqti_multitoken_cpp.bin",std::ios::binary);uint32_t count=ids.size(),hidden=c.embedding_length,vocab=r.logits[0].size();o.write((char*)&count,4);o.write((char*)&hidden,4);o.write((char*)&vocab,4);for(size_t i=0;i<ids.size();++i){o.write((char*)r.hidden[i].data(),hidden*4);o.write((char*)r.final_hidden[i].data(),hidden*4);o.write((char*)r.logits[i].data(),vocab*4);}std::cout<<"wrote "<<count<<" tokens\n";}
