#pragma once

#include <string>
#include <vector>
namespace sysmodule
{
  struct Parameter
  {
    public:
      Parameter(std::string k, std::string v) : key_(k), val_(v)
      {
          content_ = key_ + "=" + val_;
      }

      Parameter() : key_(""), val_(""), content_("") {}
      bool Empty() { return content_.empty(); }

      std::string key_;
      std::string val_;
      std::string content_;
  };

  class Parameters
  {
    public:
      Parameters();
      Parameters(const Parameters &params);
      virtual ~Parameters();
      void Append(Parameter param);
      std::vector<Parameter> GetParamters() const;
      std::string GetParamUrl() const { return param_url_; }

      bool Empty() const { return ParamGroup_.empty(); }

    private:
      std::vector<Parameter> ParamGroup_;
      std::string param_url_;
  };
}
