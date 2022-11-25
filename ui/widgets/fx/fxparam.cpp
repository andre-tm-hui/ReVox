#include "fxparam.h"
#include <iostream>

FXParam::FXParam(std::shared_ptr<FXManager> fm,
                 std::string fx,
                 std::string param,
                 QWidget *parent)
    : QWidget{parent},
      fm(fm),
      fx(fx),
      param(param)
{

}

void FXParam::setIdx(int idx) {
    this->idx = idx;
    setValue(fm->GetSetting(getPath()).get<int>());
}

void FXParam::resetIdx() {
    this->idx = -1;
    setValue(fm->GetDefaultHotkey()[fx][param].get<int>());
}

std::string FXParam::getPath() {
    return "hotkeys/" + std::to_string(idx) + "/" + fx + "/" + param;
}
