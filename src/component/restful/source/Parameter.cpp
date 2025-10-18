#include "Parameter.h"
#include <algorithm> 

namespace sysmodule
{
    Parameters::Parameters()
    {
        param_url_ = "";
    }

    Parameters::Parameters(const Parameters& params)
    {
        std::copy(ParamGroup_.begin(), ParamGroup_.end(), params.GetParamters().begin());
        param_url_ =  params.GetParamUrl();  
    }

    Parameters::~Parameters()
    {

    }

    std::vector<Parameter> Parameters::GetParamters() const
    {
        return ParamGroup_;
    }

    void Parameters::Append(Parameter param)
    {
        if (ParamGroup_.size() > 0)
        {
            param_url_ += "&";
        }
        
        param_url_ += param.content_;
        ParamGroup_.push_back(param);
    }
}