#include <node.h>
#include <jni.h>
#include <android/log.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>

#define NEWFILE O_WRONLY|O_CREAT|O_TRUNC
#define ERR -1


extern "C" {

JNIEXPORT jint Java_org_nodejs_NodeJS_start(JNIEnv *env, jobject object, jobjectArray jargv, jstring jWorkDir, jstring jLogFilePath) {
  
  const char *logFilePath = env->GetStringUTFChars(jLogFilePath, NULL);
  
  if (strlen(logFilePath) > 0) {
    
    FILE *file;
    
    if ((file = freopen(logFilePath, "a+", stdout)) == NULL) {
      
      __android_log_print(ANDROID_LOG_ERROR, "nodejs", "Failed to freopen stdout with \"%s\": %s",
        logFilePath, strerror(errno));
      return 1;
      
    }
    
    if (freopen(logFilePath, "a+", stderr) == NULL) {
      
      __android_log_print(ANDROID_LOG_ERROR, "nodejs", "Failed to freopen stderr with \"%s\":%s",
        logFilePath, strerror(errno));
      return 1;
      
    }
    
    setbuf(stdout, NULL);
    setbuf(stderr, NULL);
    
  }
  
  env->ReleaseStringUTFChars(jLogFilePath, logFilePath);
  
  int argc = env->GetArrayLength(jargv);
  char **argv = new char*[argc + 1];
  argv[argc] = NULL;
  
  size_t argvMemoryLength = 0;
  for (int i = 0; i < argc; i++) {
    jstring jstr = (jstring)env->GetObjectArrayElement(jargv, i);
    char *arg = (char*)env->GetStringUTFChars(jstr, NULL);
    size_t argLength = strlen(arg);
    argvMemoryLength += argLength + 1;
    env->ReleaseStringUTFChars(jstr, arg);
  }
  
  char *argvMemory = new char[argvMemoryLength];
  char *tmpArg = argvMemory;

  for (int i = 0; i < argc; i++) {
    
    jstring jstr = (jstring)env->GetObjectArrayElement(jargv, i);
    char *arg = (char*)env->GetStringUTFChars(jstr, NULL);
    size_t argLength = strlen(arg);
    strncpy(tmpArg, arg, argLength + 1);
    argv[i] = tmpArg;
    tmpArg += argLength + 1;
    env->ReleaseStringUTFChars(jstr, arg);
    
  }
  
  const char *workDir = env->GetStringUTFChars(jWorkDir, NULL);
  
  if (strlen(workDir) > 0) {
    
    if (chdir(workDir))
      __android_log_print(ANDROID_LOG_ERROR, "nodejs", "Failed to change working directory to \"%s\": %s",
        strerror(errno), workDir);
    
  }
  
  env->ReleaseStringUTFChars(jWorkDir, workDir);
  
  jint result = node::Start(argc, argv);
  
  delete [] argvMemory;
  delete [] argv;
  
  return result;
}

}
