# Guidance of OmafDashAccess JNI project

------

### 1. How to get libOmafDashAccess and its dependent libraries using NDK compile
```bash
git clone from the repo
cd src/external
./prebuild_android.sh
./make_android.sh
```
And then you can get libs at the following path:
| LIB name        | path   |
| --------   | -----:  |
| libglog.so     | src/build/external/android/glog/build/ |
| libssl.so        | src/build/external/android/openssl-output/lib/ |
| libcrypto.so        | src/build/external/android/openssl-output/lib/ |
| libcurl.so        | src/build/external/android/curl-output/arm64-v8a/lib/ |
| lib360SCVP.so        | src/build/android/360SCVP/ |
| libdashparser.a  | src/build/android/isolib/dash_parser/ |
| libOmafDashAccess.so | src/build/android/OmafDashAccess/ |

Copy libs to the path "src/JNIOmafDashAccess/omafdashaccesslibrary/libs/arm64-v8a/".
### 2. Prerequistes
To build the whole JNI project, there are some prerequistes must be ready.
```bash
install Android studio 3.5.1
JRE: 1.8.0_202-release-1483-b49-5587405 amd64
JVM: OpenJDK 64-Bit Server VM by JetBrains s.r.o

Android Gradle Plugion version = 3.5.1
Android Gradle version = 5.4.1
Compile SDK version = API 29: Android 10.0(Q)
Build Tools verion = 29.0.3

arm processer: arm64-v8a
```

### 3. How to run JNI project

> 1. Once the required libs and the tools mentioned above are installed, you can open the JNIOmafDashAccess project with Android Studio.
> 2. Check the android cellphone is recognized as running device.
> 3. Click 'green triangle' run button.
> 4. After the program finished, you can check a test HEVC file located at the cache path on the cellphone.
