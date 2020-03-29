/*
 * Class:     edu_cs300_MessageJNI
 * Method:    readStringMsg
 * Signature: ()Ljava/lang/String;
 */
JNIEXPORT jstring JNICALL Java_edu_cs300_MessageJNI_readStringMsg
  (JNIEnv *env, jclass ){

  }

/*
 * Class:     edu_cs300_MessageJNI
 * Method:    readPrefixRequestMsg
 * Signature: ()Ledu/cs300/SearchRequest;
 */
JNIEXPORT jobject JNICALL Java_edu_cs300_MessageJNI_readPrefixRequestMsg
  (JNIEnv *, jclass){

  }

/*
 * Class:     edu_cs300_MessageJNI
 * Method:    writeLongestWordResponseMsg
 * Signature: (ILjava/lang/String;ILjava/lang/String;Ljava/lang/String;II)V
 */
JNIEXPORT void JNICALL Java_edu_cs300_MessageJNI_writeLongestWordResponseMsg
  (JNIEnv *, jclass, jint, jstring, jint, jstring, jstring, jint, jint){

  }