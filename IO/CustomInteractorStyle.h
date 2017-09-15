#include <vtkInteractorStyleTrackballCamera.h>

class CustomInteractorStyle: public vtkInteractorStyleTrackballCamera{
    public:
        static CustomInteractorStyle* New();
        vtkTypeMacro(CustomInteractorStyle,vtkInteractorStyleTrackballCamera);

        virtual void OnKeyPress(){
            vtkRenderWindowInteractor *rwi = this->Interactor;
            std::string key = rwi->GetKeySym();
            std::cout << "Pressed " << key << std::endl;
            if (key == "s") {
                
            }
        }
}
