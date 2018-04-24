# SeetaFace_opt

#### 项目介绍
人脸探测,人脸5点定位,人脸特征提取,以及人脸识别项目.

尝试对SeetaFace项目进行性能优化.主要尝试内存优化以及GPU加速.SeetaFace项目代码来自与山世光老师的开源项目:https://github.com/seetaface/SeetaFaceEngine,本项目的代码依然遵循: BSD-2开源协议.


#### 功能介绍
山世光老师的这个项目主要包含3个部分
1:SeetaFace Detection,人脸检测,输入图片返回图片中的各个人脸所在的位置.
2:SeetaFace Alignment,人脸定位,输入图片以及人脸的位置,返回5点坐标(眼睛,鼻子,两个嘴角)
3:SeetaFace Identification,人脸识别,输入图片以及5点坐标,返回2048维特征.还提供了两个2048维特征比对相似度的接口.



#### 优化记录
1:2018-4-23
Identification:部分内存分配托管,matrix_procuct中超过200万次float运算时,使用gpu托管.

#### 优化情况
Identification 目前在i5 1050ti的gpu上缩短到了50ms左右.

```
PS H:\myOpenSource\SeetaFaceEngine-master\FaceIdentification\install\x64\bin> .\Tester.exe
Test successful!
 ExtractFeatureWithCrop time: 135.38ms
Test successful!
 CudaExtractFeatureWithCrop time: 52.7ms
```


### 编译
本项目使用vs2013进行编译,编译时需要安装cuda7以及对应的cublass.

