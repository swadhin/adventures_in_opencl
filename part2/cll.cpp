#include <stdio.h>
#include <string>
#include <iostream>

//OpenGL stuff
#include <GL/glew.h>
#if defined __APPLE__ || defined(MACOSX)
    #include <OpenGL/gl.h>
    #include <OpenGL/glext.h>
    #include <GLUT/glut.h>
    #include <OpenGL/CGLCurrent.h> 
#else
    #include <GL/glx.h>
#endif

#include "cll.h"
#include "util.h"



CL::CL()
{
    printf("Initialize OpenCL object and context\n");
    //setup devices and context
    
    std::vector<cl::Platform> platforms;
    err = cl::Platform::get(&platforms);
    printf("cl::Platform::get(): %s\n", oclErrorString(err));
    printf("platforms.size(): %d\n", platforms.size());

    deviceUsed = 0;
    err = platforms[0].getDevices(CL_DEVICE_TYPE_GPU, &devices);
    printf("getDevices: %s\n", oclErrorString(err));
    printf("devices.size(): %d\n", devices.size());
    int t = devices.front().getInfo<CL_DEVICE_TYPE>();
    printf("type: device: %d CL_DEVICE_TYPE_GPU: %d \n", t, CL_DEVICE_TYPE_GPU);

    // Define OS-specific context properties and create the OpenCL context
    //#if defined (__APPLE_CC__)
    #if defined (__APPLE__) || defined(MACOSX)
        printf("in the apple context stuff\n");
        CGLContextObj kCGLContext = CGLGetCurrentContext();
        CGLShareGroupObj kCGLShareGroup = CGLGetShareGroup(kCGLContext);
        cl_context_properties props[] =
        {
            CL_CONTEXT_PROPERTY_USE_CGL_SHAREGROUP_APPLE, (cl_context_properties)kCGLShareGroup,
            0
        };

        //this works
        //cl_context cxGPUContext = clCreateContext(props, 0, 0, NULL, NULL, &err);
        //these dont
        //cl_context cxGPUContext = clCreateContext(props, 1,(cl_device_id*)&devices.front(), NULL, NULL, &err);
        //cl_context cxGPUContext = clCreateContextFromType(props, CL_DEVICE_TYPE_GPU, NULL, NULL, &err);
        //printf("error? %s\n", oclErrorString(err));
        try{
            context = cl::Context(props);   //had to edit line 1448 of cl.hpp to add this constructor
        }
        catch (cl::Error er) {
            printf("ERROR: %s(%s)\n", er.what(), oclErrorString(er.err()));
        }
    #else
        #if defined WIN32 // Win32
            cl_context_properties props[] =
            {
                CL_GL_CONTEXT_KHR, (cl_context_properties)wglGetCurrentContext(),
                CL_WGL_HDC_KHR, (cl_context_properties)wglGetCurrentDC(),
                CL_CONTEXT_PLATFORM, (cl_context_properties)(platforms[0])(),
                0
            };
            //cl_context cxGPUContext = clCreateContext(props, 1, &cdDevices[uiDeviceUsed], NULL, NULL, &err);
            try{
                context = cl::Context(CL_DEVICE_TYPE_GPU, props);
            }
            catch (cl::Error er) {
                printf("ERROR: %s(%s)\n", er.what(), oclErrorString(er.err()));
            }
        #else
            cl_context_properties props[] =
            {
                CL_GL_CONTEXT_KHR, (cl_context_properties)glXGetCurrentContext(),
                CL_GLX_DISPLAY_KHR, (cl_context_properties)glXGetCurrentDisplay(),
                CL_CONTEXT_PLATFORM, (cl_context_properties)(platforms[0])(),
                0
            };
            //cl_context cxGPUContext = clCreateContext(props, 1, &cdDevices[uiDeviceUsed], NULL, NULL, &err);
            try{
                context = cl::Context(CL_DEVICE_TYPE_GPU, props);
            }
            catch (cl::Error er) {
                printf("ERROR: %s(%s)\n", er.what(), oclErrorString(er.err()));
            }
        #endif
    #endif

    //create the command queue we will use to execute OpenCL commands
    try{
        queue = cl::CommandQueue(context, devices[deviceUsed], 0, &err);
    }
    catch (cl::Error er) {
        printf("ERROR: %s(%d)\n", er.what(), er.err());
    }

}

CL::~CL()
{
}


void CL::loadProgram(std::string kernel_source)
{
 // Program Setup
    int pl;
    //size_t program_length;
    printf("load the program\n");
    
    pl = kernel_source.size();
    printf("kernel size: %d\n", pl);
    printf("kernel: \n %s\n", kernel_source.c_str());
    try
    {
        cl::Program::Sources source(1,
            std::make_pair(kernel_source.c_str(), pl));
   
        program = cl::Program(context, source);
    }
    catch (cl::Error er) {
        printf("ERROR: %s(%s)\n", er.what(), oclErrorString(er.err()));
    }

    printf("build program\n");
    try
    {
        err = program.build(devices);
    }
    catch (cl::Error er) {
        printf("program.build: %s\n", oclErrorString(er.err()));
    //if(err != CL_SUCCESS){
    }
    printf("done building program\n");
	std::cout << "Build Status: " << program.getBuildInfo<CL_PROGRAM_BUILD_STATUS>(devices[0]) << std::endl;
	std::cout << "Build Options:\t" << program.getBuildInfo<CL_PROGRAM_BUILD_OPTIONS>(devices[0]) << std::endl;
	std::cout << "Build Log:\t " << program.getBuildInfo<CL_PROGRAM_BUILD_LOG>(devices[0]) << std::endl;

}
