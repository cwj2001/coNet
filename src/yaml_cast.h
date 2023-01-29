//
// Created by 抑~风 on 2023/1/28.
//

#ifndef CWJ_CO_NET_YAML_CAST_H
#define CWJ_CO_NET_YAML_CAST_H

#include <string>
#include <cstring>
#include <vector>
#include <unordered_map>
#include "yaml-cpp/yaml.h"
#include "util.h"
namespace CWJ_CO_NET{

    template<typename S,typename T>
    class YamlCast{
    public:
        T operator()(const S & source){
            return static_cast<T>(source);
        }
    };

    template<>
    class YamlCast<std::string,std::string>{
    public:
        std::string operator()(const std::string & source){
            return source;
        }
    };

    template<>
    class YamlCast<YAML::Node,int>{
    public:
        int operator()(const YAML::Node & source){
            return std::stoi(source.Scalar());
        }
    };

    template<>
    class YamlCast<YAML::Node,double>{
    public:
        double operator()(const YAML::Node & source){
            return std::stod(source.Scalar());
        }
    };


    template<>
    class YamlCast<int,YAML::Node>{
    public:
        YAML::Node operator()(const int & source){
            return YAML::Node(source);
        }
    };

    template<>
    class YamlCast<double,YAML::Node>{
    public:
        YAML::Node operator()(const double & source){
            return YAML::Node(source);
        }
    };

    template<>
    class YamlCast<YAML::Node,std::string>{
    public:
        std::string operator()(const YAML::Node & source){
            return source.Scalar();
        }
    };

    template<>
    class YamlCast<std::string,YAML::Node>{
    public:
        YAML::Node operator()(const std::string & source){
            return YAML::Load(source);
        }
    };

    template<typename T>
    class YamlCast<T,std::string>{
    public:
        std::string operator()(const T& source){
            return YamlCast<T,YAML::Node>()(source).Scalar();
        }
    };

    template<typename T>
    class YamlCast<std::string,T>{
    public:
        T operator()(const std::string & source){
            return YamlCast<YAML::Node,T>()(YAML::Load(source));
        }
    };


    template<typename T>
    class YamlCast<std::vector<T>,YAML::Node>{
    public:
        YAML::Node operator()(const std::vector<T>& source){
            YAML::Node root(YAML::NodeType::Sequence);
            for(auto& a : source){

                root.template push_back(YamlCast<T,YAML::Node>()(a));
            }
            return root;
        }
    };


    template<typename T>
    class YamlCast<YAML::Node,std::vector<T>>{
    public:
       std::vector<T> operator()(const YAML::Node& source){
            std::vector<T> list;
            for(auto& a : source){
                list.push_back(std::move(YamlCast<YAML::Node,T>()(a)));
            }
            return std::move(list);
        }
    };


    template<typename V>
    class YamlCast<std::unordered_map<std::string,V>,YAML::Node>{
    public:
        YAML::Node operator()(const std::unordered_map<std::string,V>& source){
            YAML::Node root(YAML::NodeType::Map);
            for(auto& p : source){
                root[p.first] = YamlCast<V,YAML::Node>()(p.second);
            }
            return root;
        }
    };


    template<typename V>
    class YamlCast<YAML::Node,std::unordered_map<std::string,V>>{
    public:
        std::unordered_map<std::string,V> operator()(const YAML::Node& source){
           std::unordered_map<std::string,V>mp;
           for(auto & a : source){
               mp[a.first.Scalar()] = YamlCast<YAML::Node,V>()(a.second);
           }
           return std::move(mp);
        }
    };









}

#endif //CWJ_CO_NET_YAML_CAST_H
