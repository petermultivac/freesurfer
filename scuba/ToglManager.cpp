#include <stdexcept>
#include "ToglManager.h"

using namespace std;

map<ToglFrame::ID,ToglFrame*> ToglManager::mFrames;
ToglFrameFactory* ToglManager::mFactory = NULL;
InputState ToglManager::mState;


void
ToglManager::DrawCallback ( struct Togl* iTogl ) {

  ToglFrame::ID id = atoi( Togl_Ident( iTogl ));
  ToglFrame* frame = mFrames[id];
  frame->Draw();
  Togl_SwapBuffers( iTogl );
}

void
ToglManager::CreateCallback ( struct Togl* iTogl ) {

  if( NULL != mFactory ) {
    ToglFrame::ID id = atoi( Togl_Ident( iTogl ));
    ToglFrame* frame = mFactory->NewToglFrame( id );
    mFrames[id] = frame;
  }
}

void
ToglManager::DestroyCallback ( struct Togl* iTogl ) {

  char* sIdent = Togl_Ident( iTogl ); // sometimes this is null?
  if( NULL != sIdent ) {
    ToglFrame::ID id = atoi( Togl_Ident( iTogl ));
    ToglFrame* frame = mFrames[id];
    delete frame;
    mFrames[id] = NULL;
  }
}

void
ToglManager::ReshapeCallback ( struct Togl* iTogl ) {

  ToglFrame::ID id = atoi( Togl_Ident( iTogl ));
  ToglFrame* frame = mFrames[id];
  int width = Togl_Width( iTogl );
  int height = Togl_Height( iTogl );
  frame->Reshape( width, height );

  // Post a redisplay if the frame wants one. 
  if( frame->WantRedisplay() ) {
    Togl_PostRedisplay( iTogl );
    frame->RedisplayPosted();
  }
}

void
ToglManager::TimerCallback ( struct Togl* iTogl ) {

  ToglFrame::ID id = atoi( Togl_Ident( iTogl ));
  ToglFrame* frame = mFrames[id];
  frame->Timer();

  // Post a redisplay if the frame wants one. 
  if( frame->WantRedisplay() ) {
    Togl_PostRedisplay( iTogl );
    frame->RedisplayPosted();
  }
}

int
ToglManager::MouseMotionCallback ( struct Togl* iTogl, 
				   int iArgc, char* iArgv[] ) {

  // widget MouseMotionCallback x y button
  if( iArgc != 5 ) {
    return TCL_ERROR;
  }
  ToglFrame::ID id = atoi( Togl_Ident( iTogl ));
  ToglFrame* frame = mFrames[id];

  // Mouse is dragging.
  mState.mbButtonDragging = true;

  int windowCoords[2];
  windowCoords[0] = atoi(iArgv[2]);
  windowCoords[1] = YFlip(frame, atoi(iArgv[3]));
  frame->MouseMoved( windowCoords, mState );

  // Post a redisplay if the frame wants one. 
  if( frame->WantRedisplay() ) {
    Togl_PostRedisplay( iTogl );
    frame->RedisplayPosted();
  }

  return TCL_OK;
}

int
ToglManager::MouseDownCallback ( struct Togl* iTogl, 
				 int iArgc, char* iArgv[] ) {

  // widget MouseDownCallback x y button
  if( iArgc != 5 ) {
    return TCL_ERROR;
  }

  // Record this in the keyboard state.
  mState.mButton = atoi(iArgv[4]);

  // Mouse down.
  mState.mbButtonDown = true;
  mState.mbButtonDragging = false;

  ToglFrame::ID id = atoi( Togl_Ident( iTogl ));
  ToglFrame* frame = mFrames[id];

  int windowCoords[2];
  windowCoords[0] = atoi(iArgv[2]);
  windowCoords[1] = YFlip(frame, atoi(iArgv[3]));
  frame->MouseDown( windowCoords, mState );

  // Post a redisplay if the frame wants one. 
  if( frame->WantRedisplay() ) {
    Togl_PostRedisplay( iTogl );
    frame->RedisplayPosted();
  }

  return TCL_OK;
}

int
ToglManager::MouseUpCallback ( struct Togl* iTogl, int iArgc, char* iArgv[] ) {

  // widget MouseUpCallback x y button
  if( iArgc != 5 ) {
    return TCL_ERROR;
  }

  // Record this in the keyboard state.
  mState.mButton = atoi(iArgv[4]);
  
  // Mouse up.
  mState.mbButtonDown = false;
  mState.mbButtonDragging = false;

  ToglFrame::ID id = atoi( Togl_Ident( iTogl ));
  ToglFrame* frame = mFrames[id];

  int windowCoords[2];
  windowCoords[0] = atoi(iArgv[2]);
  windowCoords[1] = YFlip(frame, atoi(iArgv[3]));
  frame->MouseUp( windowCoords, mState );

  // Clear this in the keyboard state.
  mState.mButton = 0;

  // Post a redisplay if the frame wants one. 
  if( frame->WantRedisplay() ) {
    Togl_PostRedisplay( iTogl );
    frame->RedisplayPosted();
  }

  return TCL_OK;
}

int
ToglManager::KeyDownCallback ( struct Togl* iTogl, int iArgc, char* iArgv[] ) {

  // widget KeyDownCallback x y key
  if( iArgc != 5 ) {
    return TCL_ERROR;
  }

  // Look for modifiers. If it's shift (Shift_L or Shift_R), alt
  // (Alt_L or Alt_R), or ctrl (Control_L or Control_R) then just mark
  // our modifiers but don't pass the key along.
  string sKey = iArgv[4];
  if( sKey == "Shift_L" || sKey == "Shift_R" ) {
    mState.mbShiftKey = true;

  } else if( sKey == "Alt_L" || sKey == "Alt_R" ) {
    mState.mbAltKey = true;

  } else if( sKey == "Control_L" || sKey == "Control_R" ) {
    mState.mbControlKey = true;

  } else {
  
    // Record the key.
    mState.msKey = sKey;
    
    ToglFrame::ID id = atoi( Togl_Ident( iTogl ));
    ToglFrame* frame = mFrames[id];

    int windowCoords[2];
    windowCoords[0] = atoi(iArgv[2]);
    windowCoords[1] = YFlip(frame, atoi(iArgv[3]));
    frame->KeyDown( windowCoords, mState );
    
    // Post a redisplay if the frame wants one. 
    if( frame->WantRedisplay() ) {
      Togl_PostRedisplay( iTogl );
      frame->RedisplayPosted();
    }
  }

  return TCL_OK;
}

int
ToglManager::KeyUpCallback ( struct Togl* iTogl, int iArgc, char* iArgv[] ) {

   // widget KeyDownCallback x y key
 if( iArgc != 5 ) {
    return TCL_ERROR;
  }
  // Look for modifiers. If it's shift (Shift_L or Shift_R), alt
  // (Alt_L or Alt_R), or ctrl (Control_L or Control_R) then just mark
  // our modifiers but don't pass the key along.
  string sKey = iArgv[4];
  if( sKey == "Shift_L" || sKey == "Shift_R" ) {
    mState.mbShiftKey = false;

  } else if( sKey == "Alt_L" || sKey == "Alt_R" ) {
    mState.mbAltKey = false;

  } else if( sKey == "Control_L" || sKey == "Control_R" ) {
    mState.mbControlKey = false;

  } else {
  
  
    // Record the key.
    mState.msKey = sKey;
    
    ToglFrame::ID id = atoi( Togl_Ident( iTogl ));
    ToglFrame* frame = mFrames[id];

    int windowCoords[2];
    windowCoords[0] = atoi(iArgv[2]);
    windowCoords[1] = YFlip(frame, atoi(iArgv[3]));
    frame->KeyUp( windowCoords, mState );
    
    // Clear this in the keyboard state.
    mState.msKey = "";
    
    // Post a redisplay if the frame wants one.
    if( frame->WantRedisplay() ) {
      Togl_PostRedisplay( iTogl );
      frame->RedisplayPosted();
    }
  }

  return TCL_OK;
}

int
ToglManager::ExitCallback ( struct Togl* iTogl, int iArgc, char* iArgv[] ) {

  if( iArgc != 2 ) {
    return TCL_ERROR;
  }

  // Just clear the modifiers.
  mState.mbShiftKey = false;
  mState.mbAltKey = false;
  mState.mbControlKey = false;
  mState.mButton = 0;
  mState.msKey = "";
  mState.mbButtonDown = false;
  mState.mbButtonDragging = false;
  
  return TCL_OK;
}


ToglManager& 
ToglManager::GetManager() {
  static ToglManager sManager;
  return sManager;
}

void
ToglManager::InitializeTogl ( Tcl_Interp* iInterp ) {

  // Initialize the Togl module.
  int rTogl = Togl_Init( iInterp );
  if( TCL_ERROR == rTogl ) {
    throw logic_error( "Couldn't initialize Togl" );
  }

  // Register our Togl callbacks.
  Togl_CreateFunc( ToglManager::CreateCallback );
  Togl_DestroyFunc( ToglManager::DestroyCallback );
  Togl_DisplayFunc( ToglManager::DrawCallback );
  Togl_ReshapeFunc( ToglManager::ReshapeCallback );
  Togl_TimerFunc( ToglManager::TimerCallback );

  // Create our Tcl commands that will be bound to the Togl object as
  // callbacks.
  Togl_CreateCommand( "MouseMotionCallback", ToglManager::MouseMotionCallback);
  Togl_CreateCommand( "MouseDownCallback", ToglManager::MouseDownCallback );
  Togl_CreateCommand( "MouseUpCallback", ToglManager::MouseUpCallback );
  Togl_CreateCommand( "KeyDownCallback", ToglManager::KeyDownCallback );
  Togl_CreateCommand( "KeyUpCallback", ToglManager::KeyUpCallback );
  Togl_CreateCommand( "ExitCallback", ToglManager::ExitCallback );
}



ToglFrame::ToglFrame( ID iID ) {

  mID = iID;
}

ToglFrame::~ToglFrame() {

}

void 
ToglFrame::Draw() {

  this->DoDraw();
}

void 
ToglFrame::Reshape( int iWidth, int iHeight ) {

  mWidth = iWidth;
  mHeight = iHeight;

  glViewport( 0, 0, mWidth, mHeight );
  glMatrixMode( GL_PROJECTION );
  glLoadIdentity();
  glOrtho( 0, mWidth, 0, mHeight, -1.0, 1.0 );
  glMatrixMode( GL_MODELVIEW );

  this->DoReshape();
}

void 
ToglFrame::Timer() {

  this->DoTimer();
}

void
ToglFrame::MouseMoved( int iWindow[2], InputState& iInput ) {

  if( iWindow[0] > 0 && iWindow[0] < mWidth-1 &&
      iWindow[1] > 0 && iWindow[1] < mHeight-1 ) {
    this->DoMouseMoved( iWindow, iInput );
  }
}

void
ToglFrame::MouseUp( int iWindow[2], InputState& iInput ) {

  if( iWindow[0] > 0 && iWindow[0] < mWidth-1 &&
      iWindow[1] > 0 && iWindow[1] < mHeight-1 ) {
    this->DoMouseUp( iWindow, iInput );
  }
}

void
ToglFrame::MouseDown( int iWindow[2], InputState& iInput ) {

  if( iWindow[0] > 0 && iWindow[0] < mWidth-1 &&
      iWindow[1] > 0 && iWindow[1] < mHeight-1 ) {
    this->DoMouseDown( iWindow, iInput );
  }
}

void
ToglFrame::KeyDown( int iWindow[2], InputState& iInput ) {

  if( iWindow[0] > 0 && iWindow[0] < mWidth-1 &&
      iWindow[1] > 0 && iWindow[1] < mHeight-1 ) {
    this->DoKeyDown( iWindow, iInput );
  }
}

void
ToglFrame::KeyUp( int iWindow[2], InputState& iInput ) {

  if( iWindow[0] > 0 && iWindow[0] < mWidth-1 &&
      iWindow[1] > 0 && iWindow[1] < mHeight-1 ) {
    this->DoKeyUp( iWindow, iInput );
  }
}

void
ToglFrame::DoDraw() {

  DebugOutput( << "ToglFrame " << mID << ": DoDraw()" );
}

void
ToglFrame::DoReshape() {

  DebugOutput( << "ToglFrame " << mID << ": DoReshape()" );
}

void
ToglFrame::DoTimer() {

  DebugOutput( << "ToglFrame " << mID << ": DoTimer()" );
}

void
ToglFrame::DoMouseMoved( int iWindow[2], InputState& iInput ) {

  DebugOutput( << "ToglFrame " << mID << ": DoMouseMoved()" );
}

void
ToglFrame::DoMouseUp( int iWindow[2], InputState& iInput ) {

  DebugOutput( << "ToglFrame " << mID << ": DoMouseUp()" );
}

void
ToglFrame::DoMouseDown( int iWindow[2], InputState& iInput ) {

  DebugOutput( << "ToglFrame " << mID << ": DoMouseDown()" );
}

void
ToglFrame::DoKeyDown( int iWindow[2], InputState& iInput ) {

  DebugOutput( << "ToglFrame " << mID << ": DoKeyDown()" );
}

void
ToglFrame::DoKeyUp( int iWindow[2], InputState& iInput ) {

  DebugOutput( << "ToglFrame " << mID << ": DoKeyUp()" );
}
