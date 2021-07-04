//
// Created by Бушев Дмитрий on 04.07.2021.
//

#include "VulgineObjects.h"
#include "Utilities.h"

namespace Vulgine{

    void MeshImpl::createImpl() {

    }

    void MeshImpl::updateVertexData() {

    }

    void MeshImpl::updateInstanceData() {

    }

    void MeshImpl::destroyImpl() {

    }

    void RenderTargetImpl::createImpl() {
        if(renderType == SCREEN){
            return;
        }
        else{
            Utilities::ExitFatal(-1, "Offscreen render targets aren't supported yet");
        }
    }

    void RenderTargetImpl::destroyImpl() {

    }

    void MaterialImpl::createImpl() {

    }

    void MaterialImpl::destroyImpl() {

    }

    void LightImpl::createImpl() {

    }

    void LightImpl::destroyImpl() {

    }
}
