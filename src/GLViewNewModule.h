#pragma once

#include "GLView.h"
#include <irrKlang.h>

namespace Aftr {
    class Camera;

    class GLViewNewModule : public GLView {
        public:
            int background = 0;
            static GLViewNewModule* New( const std::vector< std::string >& outArgs );
            virtual ~GLViewNewModule();
            virtual void init(float gravityScalar, Vector gravityNormalizedVector, std::string confFileName, const PHYSICS_ENGINE_TYPE& physicsEngineType);
            virtual void updateWorld(); ///< Called once per frame
            virtual void loadMap(); ///< Called once at startup to build this module's scene
            virtual void createNewModuleWayPoints();
            virtual void onResizeWindow( GLsizei width, GLsizei height );
            virtual void onMouseDown( const SDL_MouseButtonEvent& e );
            virtual void onMouseUp( const SDL_MouseButtonEvent& e );
            virtual void onMouseMove( const SDL_MouseMotionEvent& e );
            virtual void onKeyDown( const SDL_KeyboardEvent& key );
            virtual void onKeyUp( const SDL_KeyboardEvent& key );
            virtual std::string getBackground();
            virtual void hummerMove(float dist = 0.1f);
            bool autoPilot = false;
            bool followHummer = false;

        protected:
            GLViewNewModule( const std::vector< std::string >& args );
            virtual void onCreate();
            
            WO* hummer;
    };
}

