#include "GLViewNewModule.h"

#include "WorldList.h" //This is where we place all of our WOs
#include "ManagerOpenGLState.h" //We can change OpenGL State attributes with this
#include "Axes.h" //We can set Axes to on/off with this
#include "PhysicsEngineODE.h"

//Different WO used by this module
#include "WO.h"
#include "WOStatic.h"
#include "WOStaticPlane.h"
#include "WOStaticTrimesh.h"
#include "WOTrimesh.h"
#include "WOHumanCyborg.h"
#include "WOHumanCal3DPaladin.h"
#include "WOWayPointSpherical.h"
#include "WOLight.h"
#include "WOSkyBox.h"
#include "WOCar1970sBeater.h"
#include "Camera.h"
#include "CameraStandard.h"
#include "CameraChaseActorSmooth.h"
#include "CameraChaseActorAbsNormal.h"
#include "CameraChaseActorRelNormal.h"
#include "Model.h"
#include "ModelDataShared.h"
#include "ModelMesh.h"
#include "ModelMeshDataShared.h"
#include "ModelMeshSkin.h"
#include "WONVStaticPlane.h"
#include "WONVPhysX.h"
#include "WONVDynSphere.h"
#include "AftrGLRendererBase.h"
#include <irrKlang.h>
#include "ISoundManager.h"

//If we want to use way points, we need to include this.
#include "NewModuleWayPoints.h"

using namespace Aftr;
using namespace irrklang;

ISound* citySound = nullptr;
ISound* hummerSound = nullptr;
ISound* horn = nullptr;
SoundEndListener* soundEndListener;

GLViewNewModule* GLViewNewModule::New( const std::vector< std::string >& args ){   
    GLViewNewModule* glv = new GLViewNewModule( args );
    glv->init( Aftr::GRAVITY, Vector( 0, 0, -1.0f ), "aftr.conf", PHYSICS_ENGINE_TYPE::petODE );
    glv->onCreate();
    return glv;
}

void GLViewNewModule::init(float gravityScalar, Vector gravityNormalizedVector, std::string configFileName, const PHYSICS_ENGINE_TYPE& physicsEngineType) {
    ISoundManager::init();
    GLView::init(gravityScalar, gravityNormalizedVector, configFileName, physicsEngineType);
}


GLViewNewModule::GLViewNewModule( const std::vector< std::string >& args ) : GLView( args ){
    //Initialize any member variables that need to be used inside of LoadMap() here.
    //Note: At this point, the Managers are not yet initialized. The Engine initialization
    //occurs immediately after this method returns (see GLViewNewModule::New() for
    //reference). Then the engine invoke's GLView::loadMap() for this module.
    //After loadMap() returns, GLView::onCreate is finally invoked.

    //The order of execution of a module startup:
    //GLView::New() is invoked:
    //    calls GLView::init()
    //       calls GLView::loadMap() (as well as initializing the engine's Managers)
    //    calls GLView::onCreate()
    
    //GLViewNewModule::onCreate() is invoked after this module's LoadMap() is completed.
}

void GLViewNewModule::onCreate(){
    //GLViewNewModule::onCreate() is invoked after this module's LoadMap() is completed.
    //At this point, all the managers are initialized. That is, the engine is fully initialized.

    if( this->pe != NULL ){
        //optionally, change gravity direction and magnitude here
        //The user could load these values from the module's aftr.conf
        this->pe->setGravityNormalizedVector( Vector( 0,0,-1.0f ) );
        this->pe->setGravityScalar( Aftr::GRAVITY );
    }
    this->setActorChaseType( STANDARDEZNAV ); //Default is STANDARDEZNAV mode
    //this->setNumPhysicsStepsPerRender( 0 ); //pause physics engine on start up; will remain paused till set to 1
 
    this->hummer = WO::New(ManagerEnvironmentConfiguration::getSMM() + "/models/WOCarHummerTruck.wrl", Vector(2, 2, 2));
    this->hummer->setLabel("Hummer");
    this->hummer->setPosition(Vector(13, -17, -6));
    this->hummer->renderOrderType = RENDER_ORDER_TYPE::roLIGHT;
    worldLst->push_back(this->hummer);

    soundEndListener = new SoundEndListener();

    citySound = ISoundManager::getEngine()->play2D("../mm/sounds/citysounds.ogg", true);
    hummerSound = ISoundManager::getEngine()->play3D("../mm/sounds/carsounds.wav", ISoundManager::toVec3df(this->hummer->getPosition()), true, false, true);
    horn = ISoundManager::getEngine()->play3D("../mm/sounds/horn1.wav", ISoundManager::toVec3df(this->hummer->getPosition()), false, true, true);
    horn->setSoundStopEventReceiver(soundEndListener);
    hummerSound->setMinDistance(10);
}


GLViewNewModule::~GLViewNewModule(){
    //Implicitly calls GLView::~GLView()
    ISoundManager::drop();
}


void GLViewNewModule::updateWorld(){
    GLView::updateWorld(); //Just call the parent's update world first.
                          //If you want to add additional functionality, do it after this call.
    ISoundManager::setListenerPosition(this->cam->getPosition(), this->cam->getLookDirection(), Vector(0, 0, 0), this->cam->getNormalDirection());
    if (this->autoPilot) {
        hummerMove(1);
    }
    if (hummerSound != nullptr) {
        hummerSound->setPosition(ISoundManager::toVec3df(this->hummer->getPosition()));
    }
    if (horn != nullptr) {
        horn->setPosition(ISoundManager::toVec3df(this->hummer->getPosition()));
    }
}

void GLViewNewModule::onResizeWindow( GLsizei width, GLsizei height ){ GLView::onResizeWindow( width, height ); }
void GLViewNewModule::onMouseDown( const SDL_MouseButtonEvent& e ){ GLView::onMouseDown( e ); }
void GLViewNewModule::onMouseUp( const SDL_MouseButtonEvent& e ){ GLView::onMouseUp( e ); }
void GLViewNewModule::onMouseMove( const SDL_MouseMotionEvent& e ){ GLView::onMouseMove( e );}

void GLViewNewModule::onKeyDown( const SDL_KeyboardEvent& key ){
    GLView::onKeyDown( key );
    SDL_Keycode pressed = key.keysym.sym;
    if (pressed == SDLK_0){
        this->setNumPhysicsStepsPerRender(1);
    }
    if(pressed == SDLK_1 ){}
    if (pressed == SDLK_F9){
       loadMap();
       std::cout << "Reloaded world with new background!" << std::endl;
    }
    if ((pressed == SDLK_w || pressed == SDLK_UP) && this->cam != nullptr) {
        this->cam->moveInLookDirection(10);
    }
    if ((pressed == SDLK_s || pressed == SDLK_DOWN) && this->cam != nullptr) {
        this->cam->moveOppositeLookDirection(10);
    }
    if ((pressed == SDLK_a || pressed == SDLK_LEFT) && this->cam != nullptr) {
        this->cam->moveLeft();
        this->cam->moveLeft();
        this->cam->moveLeft();
        this->cam->moveLeft();
        this->cam->moveLeft();
        this->cam->moveLeft();
    }
    if ((pressed == SDLK_d || pressed == SDLK_RIGHT) && this->cam != nullptr) {
        this->cam->moveRight();
        this->cam->moveRight();
        this->cam->moveRight();
        this->cam->moveRight();
        this->cam->moveRight();
        this->cam->moveRight();
    }
    if (pressed == SDLK_SPACE && this->hummer != nullptr && horn != nullptr) {
        if (horn->isFinished()) {
            //horn->drop();
            horn = ISoundManager::getEngine()->play3D("../mm/sounds/horn1.wav", ISoundManager::toVec3df(this->hummer->getPosition()), false, true, true);
            horn->setSoundStopEventReceiver(soundEndListener);
        }
        horn->setIsPaused(false);
    }
    if (pressed == SDLK_o) {
        ISoundManager::getEngine()->play2D("../mm/sounds/oof.mp3");
    }
    if (pressed == SDLK_l && this->cam != nullptr) {
        std::cout << "Position: " << this->cam->getPosition() << std::endl;
    }
    if (pressed == SDLK_t && this->cam != nullptr) {
        std::cout << "Cam Position: " << this->cam->getPosition() << std::endl;
        vec3df vec = ISoundManager::toVec3df(this->cam->getPosition());
        std::cout << "Sound Position: " << vec.X << " " << vec.Y << " " << vec.Z << std::endl;
        ISoundManager::getEngine()->play3D("../mm/sounds/oof.mp3", vec);
    }
    if (pressed == SDLK_0 && this->cam != nullptr) {
        this->cam->setPosition(Vector(0, 0, 0));
    }
    if (pressed == SDLK_m) {
        if (!this->autoPilot) {
            this->autoPilot = true;
        } else {
            this->autoPilot = false;
        }
    }
    if (pressed == SDLK_f) {
        if (!this->followHummer) {
            this->followHummer = true;
            if (this->hummer != nullptr && this->cam != nullptr) {
                Vector hummerPos = this->hummer->getPosition();
                this->cam->setPosition(hummerPos.x - 5, hummerPos.y, hummerPos.z + 5);
            }
        } else {
            this->followHummer = false;
        }
    }
}

void GLViewNewModule::hummerMove(float distance) {
    if (this->hummer != nullptr) {
        Vector hummerPos = this->hummer->getPosition();
        this->hummer->setPosition(Vector(hummerPos.x + distance, hummerPos.y, hummerPos.z));
        if (this->hummer->getPosition().x > 1700) {
            this->hummer->setPosition(Vector(13, -17, -6));
        }
        hummerPos = this->hummer->getPosition();
        if (this->followHummer && this->hummer != nullptr && this->cam != nullptr) {       
            this->cam->setPosition(hummerPos.x - 5, hummerPos.y, hummerPos.z + 5);
        }
    }
}

void GLViewNewModule::onKeyUp( const SDL_KeyboardEvent& key ){
    GLView::onKeyUp( key );
}

void Aftr::GLViewNewModule::loadMap(){
    std::string sharedMM = ManagerEnvironmentConfiguration::getSMM();

    this->worldLst = new WorldList(); //WorldList is a 'smart' vector that is used to store WO*'s
    this->actorLst = new WorldList();
    this->netLst = new WorldList();

    ManagerOpenGLState::GL_CLIPPING_PLANE = 1000.0;
    ManagerOpenGLState::GL_NEAR_PLANE = 0.1f;
    ManagerOpenGLState::enableFrustumCulling = false;
    Axes::isVisible = true;
    this->glRenderer->isUsingShadowMapping( false ); //set to TRUE to enable shadow mapping, must be using GL 3.2+

    this->cam->setPosition( -25, 0, 15 );

   // std::string shinyRedPlasticCube( sharedMM + "/models/cube4x4x4redShinyPlastic_pp.wrl" );
    
    std::string city(sharedMM + "/models/citytexday_3ds/city_tex_day.3DS");
    std::string wheeledCar(sharedMM + "/models/rcx_treads.wrl" );
    std::string grass(sharedMM + "/models/grassFloor400x400_pp.wrl" );
    //std::string grass(sharedMM + "/models/landscapeSeabed.wrl");
    std::string human(sharedMM + "/models/human_chest.wrl" );
   
    //SkyBox Textures readily available
    std::vector< std::string > skyBoxImageNames; //vector to store texture paths

    skyBoxImageNames.push_back(sharedMM + getBackground());
    ++background;

    float ga = 0.1f; //Global Ambient Light level for this module
    ManagerLight::setGlobalAmbientLight(aftrColor4f(ga, ga, ga, 1.0f));
    WOLight* light = WOLight::New();
    light->isDirectionalLight(true);
    light->setPosition(Vector(0, 0, 100));
    //Set the light's display matrix such that it casts light in a direction parallel to the -z axis (ie, downwards as though it was "high noon")
    //for shadow mapping to work, this->glRenderer->isUsingShadowMapping( true ), must be invoked.
    light->getModel()->setDisplayMatrix( Mat4::rotateIdentityMat({ 0, 1, 0 }, 90.0f * Aftr::DEGtoRAD));
    light->setLabel( "Light" );
    worldLst->push_back( light );
    
    //Create the SkyBox
    WO* wo = WOSkyBox::New( skyBoxImageNames.at( 0 ), this->getCameraPtrPtr() );
    wo->setPosition(Vector(0, 0, 0));
    wo->setLabel("Sky Box");
    wo->renderOrderType = RENDER_ORDER_TYPE::roOPAQUE;
    worldLst->push_back(wo);
    
    ////Create the infinite grass plane (the floor)
   /* wo = WO::New( grass, Vector( 1, 1, 1 ), MESH_SHADING_TYPE::mstFLAT );
    wo->setPosition( Vector( 0, 0, 0 ) );
    wo->renderOrderType = RENDER_ORDER_TYPE::roOPAQUE;
    ModelMeshSkin& grassSkin = wo->getModel()->getModelDataShared()->getModelMeshes().at( 0 )->getSkins().at( 0 );
    grassSkin.getMultiTextureSet().at( 0 )->setTextureRepeats( 5.0f );
    grassSkin.setAmbient( aftrColor4f( 0.4f, 0.4f, 0.4f, 1.0f ) ); //Color of object when it is not in any light
    grassSkin.setDiffuse( aftrColor4f( 1.0f, 1.0f, 1.0f, 1.0f ) ); //Diffuse color components (ie, matte shading color of this object)
    grassSkin.setSpecular( aftrColor4f( 0.4f, 0.4f, 0.4f, 1.0f ) ); //Specular color component (ie, how "shiney" it is)
    grassSkin.setSpecularCoefficient( 10 ); // How "sharp" are the specular highlights (bigger is sharper, 1000 is very sharp, 10 is very dull)
    wo->setLabel( "Grass" );
    worldLst->push_back( wo );*/
    
    ////Create the infinite grass plane that uses the Open Dynamics Engine (ODE)
    //wo = WOStatic::New( grass, Vector(1,1,1), MESH_SHADING_TYPE::mstFLAT );
    //((WOStatic*)wo)->setODEPrimType( ODE_PRIM_TYPE::PLANE );
    //wo->setPosition( Vector(0,0,0) );
    //wo->renderOrderType = RENDER_ORDER_TYPE::roOPAQUE;
    //wo->getModel()->getModelDataShared()->getModelMeshes().at(0)->getSkins().at(0).getMultiTextureSet().at(0)->setTextureRepeats( 5.0f );
    //wo->setLabel( "Grass" );
    //worldLst->push_back( wo );
    
    ////Create the infinite grass plane that uses NVIDIAPhysX(the floor)
    //wo = WONVStaticPlane::New( grass, Vector(1,1,1), MESH_SHADING_TYPE::mstFLAT );
    //wo->setPosition( Vector(0,0,0) );
    //wo->renderOrderType = RENDER_ORDER_TYPE::roOPAQUE;
    //wo->getModel()->getModelDataShared()->getModelMeshes().at(0)->getSkins().at(0).getMultiTextureSet().at(0)->setTextureRepeats( 5.0f );
    //wo->setLabel( "Grass" );
    //worldLst->push_back( wo );
    
    ////Create the infinite grass plane (the floor)
    //wo = WO::New( shinyRedPlasticCube, Vector(1,1,1), MESH_SHADING_TYPE::mstFLAT );
    //wo->setPosition( Vector(0,0,20.0f) );
    //wo->renderOrderType = RENDER_ORDER_TYPE::roOPAQUE;
    //wo->setLabel( "Shiny Red Plastic Cube" );
    //worldLst->push_back( wo );
    wo = WO::New(city, Vector(1, 1, 1), MESH_SHADING_TYPE::mstAUTO);
    wo->setPosition(Vector(0, 0, 1350.0f));
    wo->renderOrderType = RENDER_ORDER_TYPE::roLIGHT;
    wo->setLabel("City");
    worldLst->push_back(wo);
    
    //wo = WONVPhysX::New( shinyRedPlasticCube, Vector(1,1,1), MESH_SHADING_TYPE::mstFLAT );
    //wo->setPosition( Vector(0,0.5f,75.0f) );
    //wo->renderOrderType = RENDER_ORDER_TYPE::roOPAQUE;
    //wo->setLabel( "Grass" );
    //worldLst->push_back( wo );
    
    //wo = WONVDynSphere::New( ManagerEnvironmentConfiguration::getVariableValue("sharedmultimediapath") + "/models/sphereRp5.wrl", Vector(1.0f, 1.0f, 1.0f), mstSMOOTH );
    //wo->setPosition( 0,0,100.0f );
    //wo->setLabel( "Sphere" );
    //this->worldLst->push_back( wo );
    
    //wo = WOHumanCal3DPaladin::New( Vector( .5, 1, 1 ), 100 );
    //((WOHumanCal3DPaladin*)wo)->rayIsDrawn = false; //hide the "leg ray"
    //((WOHumanCal3DPaladin*)wo)->isVisible = false; //hide the Bounding Shell
    //wo->setPosition( Vector(20,20,20) );
    //wo->setLabel( "Paladin" );
    //worldLst->push_back( wo );
    //actorLst->push_back( wo );
    //netLst->push_back( wo );
    //this->setActor( wo );
    //
    //wo = WOHumanCyborg::New( Vector( .5, 1.25, 1 ), 100 );
    //wo->setPosition( Vector(20,10,20) );
    //wo->isVisible = false; //hide the WOHuman's bounding box
    //((WOHuman*)wo)->rayIsDrawn = false; //show the 'leg' ray
    //wo->setLabel( "Human Cyborg" );
    //worldLst->push_back( wo );
    //actorLst->push_back( wo ); //Push the WOHuman as an actor
    //netLst->push_back( wo );
    //this->setActor( wo ); //Start module where human is the actor
    
    ////Create and insert the WOWheeledVehicle
    //std::vector< std::string > wheels;
    //std::string wheelStr( "../../../shared/mm/models/WOCar1970sBeaterTire.wrl" );
    //wheels.push_back( wheelStr );
    //wheels.push_back( wheelStr );
    //wheels.push_back( wheelStr );
    //wheels.push_back( wheelStr );
    //wo = WOCar1970sBeater::New( "../../../shared/mm/models/WOCar1970sBeater.wrl", wheels );
    //wo->setPosition( Vector( 5, -15, 20 ) );
    //wo->setLabel( "Car 1970s Beater" );
    //((WOODE*)wo)->mass = 200;
    //worldLst->push_back( wo );
    //actorLst->push_back( wo );
    //this->setActor( wo );
    //netLst->push_back( wo );

    createNewModuleWayPoints();
}

void GLViewNewModule::createNewModuleWayPoints(){
   // Create a waypoint with a radius of 3, a frequency of 5 seconds, activated by GLView's camera, and is visible.
   WayPointParametersBase params(this);
   params.frequency = 5000;
   params.useCamera = true;
   params.visible = true;
   WOWayPointSpherical* wayPt = WOWP1::New( params, 3 );
   wayPt->setPosition( Vector( 50, 0, 3 ) );
   worldLst->push_back( wayPt );
}

std::string GLViewNewModule::getBackground() {
    switch (background) {
    case 0: return "/images/skyboxes/sky_water+6.jpg";
    case 1: return "/images/skyboxes/sky_dust+6.jpg";
    case 2: return "/images/skyboxes/sky_mountains+6.jpg";
    case 3: return "/images/skyboxes/sky_winter+6.jpg";
    case 4: return "/images/skyboxes/early_morning+6.jpg";
    case 5: return "/images/skyboxes/sky_afternoon+6.jpg";
    case 6: return "/images/skyboxes/sky_cloudy+6.jpg";
    case 7: return "/images/skyboxes/sky_cloudy3+6.jpg";
    case 8: return "/images/skyboxes/sky_day+6.jpg";
    case 9: return "/images/skyboxes/sky_day2+6.jpg";
    case 10: return "/images/skyboxes/sky_deepsun+6.jpg";
    case 11: return "/images/skyboxes/sky_evening+6.jpg";
    case 12: return "/images/skyboxes/sky_morning+6.jpg";
    case 13: return "/images/skyboxes/sky_morning2+6.jpg";
    case 14: return "/images/skyboxes/sky_noon+6.jpg";
    case 15: return "/images/skyboxes/sky_warp+6.jpg";
    case 16: return "/images/skyboxes/space_Hubble_Nebula+6.jpg";
    case 17: return "/images/skyboxes/space_gray_matter+6.jpg";
    case 18: return "/images/skyboxes/space_easter+6.jpg";
    case 19: return "/images/skyboxes/space_hot_nebula+6.jpg";
    case 20: return "/images/skyboxes/space_ice_field+6.jpg";
    case 21: return "/images/skyboxes/space_lemon_lime+6.jpg";
    case 22: return "/images/skyboxes/space_milk_chocolate+6.jpg";
    case 23: return "/images/skyboxes/space_solar_bloom+6.jpg";
    case 24: return "/images/skyboxes/space_thick_rb+6.jpg";
    default:
        background = 0;
        return getBackground();
    }
}