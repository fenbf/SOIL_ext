//
//  main.cpp
//  SOIL osx test
//
//  Created by Robert Bu on 8/31/13.
//  Copyright (c) 2013 Robert Bu. All rights reserved.
//

#include <iostream>
#include <Cocoa/Cocoa.h>
#include "SOIL.h"

@interface AppDelegate : NSObject <NSApplicationDelegate> {
    void (^drawCallback)(void);
}

@property NSWindow* window;
@property NSOpenGLContext* glContext;

- (void)setDrawCallback:(id)callback;

@end

@implementation AppDelegate

- (void)draw {    
    [self.glContext makeCurrentContext];
    
    if(self->drawCallback) {
        (self->drawCallback)();
    }
    
    [self.glContext flushBuffer];
}

- (void)applicationDidFinishLaunching:(NSNotification *)aNotification {
    self.window = [self createWindow];
    self.glContext = [self createGLContext];
    
    [self.glContext setView:self.window.contentView];
    
    [NSTimer scheduledTimerWithTimeInterval:.1
                                     target:self
                                   selector:@selector(draw)
                                   userInfo:nil
                                    repeats:YES];
}

- (void)setDrawCallback:(id)callback {
    self->drawCallback = callback;
}

- (BOOL)applicationShouldTerminateAfterLastWindowClosed:(NSApplication *)_app {
    return YES;
}

- (NSOpenGLContext*) createGLContext {
    NSOpenGLPixelFormatAttribute attrs[] =
    {
        NSOpenGLPFADoubleBuffer,
        NSOpenGLPFADepthSize, 32,
        0
    };
    
    NSOpenGLPixelFormat* pixFmt = [[NSOpenGLPixelFormat alloc] initWithAttributes:attrs];
    if(!pixFmt)
        return 0;
    
    NSOpenGLContext* ctx = [[NSOpenGLContext alloc] initWithFormat:pixFmt shareContext:0];
    return ctx;
}

- (NSWindow*) createWindow {
    NSWindow* wnd = [[NSWindow alloc] initWithContentRect:NSMakeRect(0, 0, 800, 600)
                                                styleMask:NSTitledWindowMask | NSClosableWindowMask |NSMiniaturizableWindowMask
                                                  backing:NSBackingStoreBuffered defer:NO];
    
    [wnd.contentView setWantsBestResolutionOpenGLSurface:YES];
    [wnd makeKeyAndOrderFront:nil];
    [wnd center];
    
    return wnd;
}


@end

GLuint loadTex(const char* cmd) {
    glEnable( GL_BLEND );
    //glDisable( GL_BLEND );
    //	straight alpha
    glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
    //	premultiplied alpha (remember to do the same in glColor!!)
    //glBlendFunc( GL_ONE, GL_ONE_MINUS_SRC_ALPHA );
    
    //	do I want alpha thresholding?
    glEnable( GL_ALPHA_TEST );
    glAlphaFunc( GL_GREATER, 0.5f );
    
    //	log what the use is asking us to load
    std::string load_me = cmd;
    if( load_me.length() > 2 )
    {
		//load_me = load_me.substr( 1, load_me.length() - 2 );
		load_me = load_me.substr( 0, load_me.length() - 0 );
    } else
    {
    	//load_me = "img_test_uncompressed.dds";
    	//load_me = "img_test_indexed.tga";
    	//load_me = "img_test.dds";
    	load_me = "img_test.png";
    	//load_me = "odd_size.jpg";
    	//load_me = "img_cheryl.jpg";
    	//load_me = "oak_odd.png";
    	//load_me = "field_128_cube.dds";
    	//load_me = "field_128_cube_nomip.dds";
    	//load_me = "field_128_cube_uc.dds";
    	//load_me = "field_128_cube_uc_nomip.dds";
    	//load_me = "Goblin.dds";
    	//load_me = "parquet.dds";
    	//load_me = "stpeters_probe.hdr";
    	//load_me = "VeraMoBI_sdf.png";
        
    	//	for testing the texture rectangle code
    	//load_me = "test_rect.png";
    }
	std::cout << "'" << load_me << "'" << std::endl;
    
	//	1st try to load it as a single-image-cubemap
	//	(note, need DDS ordered faces: "EWUDNS")
	GLuint tex_ID;
    int time_me;
    
    std::cout << "Attempting to load as a cubemap" << std::endl;
    time_me = clock();
	tex_ID = SOIL_load_OGL_single_cubemap(load_me.c_str(),
                                          SOIL_DDS_CUBEMAP_FACE_ORDER,
                                          SOIL_LOAD_AUTO,
                                          SOIL_CREATE_NEW_ID,
                                          SOIL_FLAG_POWER_OF_TWO
                                          | SOIL_FLAG_MIPMAPS
                                          //| SOIL_FLAG_COMPRESS_TO_DXT
                                          //| SOIL_FLAG_TEXTURE_REPEATS
                                          //| SOIL_FLAG_INVERT_Y
                                          | SOIL_FLAG_DDS_LOAD_DIRECT
                                          );
	time_me = clock() - time_me;
	std::cout << "the load time was " << 0.001f * time_me << " seconds (warning: low resolution timer)" << std::endl;
    if( tex_ID > 0 )
    {
    	glEnable( GL_TEXTURE_CUBE_MAP );
		glEnable( GL_TEXTURE_GEN_S );
		glEnable( GL_TEXTURE_GEN_T );
		glEnable( GL_TEXTURE_GEN_R );
		glTexGeni( GL_S, GL_TEXTURE_GEN_MODE, GL_REFLECTION_MAP );
		glTexGeni( GL_T, GL_TEXTURE_GEN_MODE, GL_REFLECTION_MAP );
		glTexGeni( GL_R, GL_TEXTURE_GEN_MODE, GL_REFLECTION_MAP );
		glBindTexture( GL_TEXTURE_CUBE_MAP, tex_ID );
		//	report
		std::cout << "the loaded single cube map ID was " << tex_ID << std::endl;
		//std::cout << "the load time was " << 0.001f * time_me << " seconds (warning: low resolution timer)" << std::endl;
    } else
    {
    	std::cout << "Attempting to load as a HDR texture" << std::endl;
		time_me = clock();
		tex_ID = SOIL_load_OGL_HDR_texture(
                                           load_me.c_str(),
                                           //SOIL_HDR_RGBE,
                                           //SOIL_HDR_RGBdivA,
                                           SOIL_HDR_RGBdivA2,
                                           0,
                                           SOIL_CREATE_NEW_ID,
                                           SOIL_FLAG_POWER_OF_TWO
                                           | SOIL_FLAG_MIPMAPS
                                           //| SOIL_FLAG_COMPRESS_TO_DXT
                                           );
		time_me = clock() - time_me;
		std::cout << "the load time was " << 0.001f * time_me << " seconds (warning: low resolution timer)" << std::endl;
        
		//	did I fail?
		if( tex_ID < 1 )
		{
			//	loading of the single-image-cubemap failed, try it as a simple texture
			std::cout << "Attempting to load as a simple 2D texture" << std::endl;
			//	load the texture, if specified
			time_me = clock();
			tex_ID = SOIL_load_OGL_texture(
                                           load_me.c_str(),
                                           SOIL_LOAD_AUTO,
                                           SOIL_CREATE_NEW_ID,
                                           /*SOIL_FLAG_POWER_OF_TWO*/
                                           SOIL_FLAG_GL_MIPMAPS
                                           //| SOIL_FLAG_MULTIPLY_ALPHA
                                           | SOIL_FLAG_COMPRESS_TO_DXT
                                           | SOIL_FLAG_DDS_LOAD_DIRECT
                                           //| SOIL_FLAG_NTSC_SAFE_RGB
                                           //| SOIL_FLAG_CoCg_Y
                                           //| SOIL_FLAG_TEXTURE_RECTANGLE
                                           );
			time_me = clock() - time_me;
			std::cout << "the load time was " << 0.001f * time_me << " seconds (warning: low resolution timer)" << std::endl;
		}
        
		if( tex_ID > 0 )
		{
			//	enable texturing
			glEnable( GL_TEXTURE_2D );
			//glEnable( 0x84F5 );// enables texture rectangle
			//  bind an OpenGL texture ID
			glBindTexture( GL_TEXTURE_2D, tex_ID );
			//	report
			std::cout << "the loaded texture ID was " << tex_ID << std::endl;
			//std::cout << "the load time was " << 0.001f * time_me << " seconds (warning: low resolution timer)" << std::endl;
		} else
		{
			//	loading of the texture failed...why?
			glDisable( GL_TEXTURE_2D );
			std::cout << "Texture loading failed: '" << SOIL_last_result() << "'" << std::endl;
		}
        
        return tex_ID;
    }
    return 0;
}

int runWithDrawCallback(void (^callback)(void)) {
    @autoreleasepool {
        
        NSApplication* app = [NSApplication sharedApplication];
        AppDelegate* delegate = [[AppDelegate alloc] init];
        [app setDelegate:delegate];
        
        [delegate setDrawCallback:callback];
        
        ProcessSerialNumber psn = { 0, kCurrentProcess };
        TransformProcessType(&psn, kProcessTransformToForegroundApplication);
        SetFrontProcess(&psn);
        
        
        [app run];
        
    }
    return EXIT_SUCCESS;
}

int main(int argc, const char * argv[])
{
    __block float theta = 0.f;
    __block GLuint tex = 0;
    __block bool saved = false;
    
    return runWithDrawCallback(^() {
        if(!tex) {
            tex = loadTex(argc > 1 ? argv[1]: "");
        }
        
        glClearColor(1, 0, 1, 1);
        glClear(GL_COLOR_BUFFER_BIT);
        
        const float ref_mag = 0.1f;
        // OpenGL animation code goes here
        theta = clock() * 0.1f;
        
        float tex_u_max = 1.0f;//0.2f;
        float tex_v_max = 1.0f;//0.2f;
        
        glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        
        glPushMatrix();
        glScalef( 0.8f, 0.8f, 0.8f );
        //glRotatef(-0.314159f*theta, 0.0f, 0.0f, 1.0f);
        glColor4f( 1.0f, 1.0f, 1.0f, 1.0f );
        glNormal3f( 0.0f, 0.0f, 1.0f );
        glBegin(GL_QUADS);
        glNormal3f( -ref_mag, -ref_mag, 1.0f );
        glTexCoord2f( 0.0f, tex_v_max );
        glVertex3f( -1.0f, -1.0f, -0.1f );
        
        glNormal3f( ref_mag, -ref_mag, 1.0f );
        glTexCoord2f( tex_u_max, tex_v_max );
        glVertex3f( 1.0f, -1.0f, -0.1f );
        
        glNormal3f( ref_mag, ref_mag, 1.0f );
        glTexCoord2f( tex_u_max, 0.0f );
        glVertex3f( 1.0f, 1.0f, -0.1f );
        
        glNormal3f( -ref_mag, ref_mag, 1.0f );
        glTexCoord2f( 0.0f, 0.0f );
        glVertex3f( -1.0f, 1.0f, -0.1f );
        glEnd();
        glPopMatrix();
        
        tex_u_max = 1.0f;
        tex_v_max = 1.0f;
        glPushMatrix();
        glScalef( 0.8f, 0.8f, 0.8f );
        glRotatef(theta, 0.0f, 0.0f, 1.0f);
        glColor4f( 1.0f, 1.0f, 1.0f, 1.0f );
        glNormal3f( 0.0f, 0.0f, 1.0f );
        glBegin(GL_QUADS);
        glTexCoord2f( 0.0f, tex_v_max );		glVertex3f( 0.0f, 0.0f, 0.1f );
        glTexCoord2f( tex_u_max, tex_v_max );		glVertex3f( 1.0f, 0.0f, 0.1f );
        glTexCoord2f( tex_u_max, 0.0f );		glVertex3f( 1.0f, 1.0f, 0.1f );
        glTexCoord2f( 0.0f, 0.0f );		glVertex3f( 0.0f, 1.0f, 0.1f );
        glEnd();
        glPopMatrix();
        
        {
            /*	check for errors	*/
            GLenum err_code = glGetError();
            while( GL_NO_ERROR != err_code )
            {
                printf( "OpenGL Error @ %s: %i", "drawing loop", err_code );
                err_code = glGetError();
            }
        }

        glFlush();
        
        if(!saved) {
            SOIL_save_screenshot("screenshot.bmp", SOIL_SAVE_TYPE_BMP, 0, 0, 800, 600);
            saved = true;
        }
    });
    
}

