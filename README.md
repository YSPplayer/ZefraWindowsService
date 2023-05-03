# 个人使用的Windows服务
  ●运行在windows后台的服务程序，主要做一些服务功能，支持开机自启动。
  ## 服务功能详情
  ### 1.替换windows桌面壁纸
  ①：一定时间内随机替换windows的桌面壁纸  
  ②：在主文件夹下的`config.json-Service`中可以配置替换壁纸的属性，具体配置属性如下  
  ●`SwitchingTime`:壁纸切换的时间，以毫秒为单位  
  ●`bgPath`:壁纸加载的路径  
  ●`openService`:是否把服务写入注册表？为false则删除写入的注册表，下次的开机不再自启动该服务  
  ## Release食用方法  
  ①：点击主目录下的`run.bat`文件运行即可，下次开机不再需要手动运行。
  ②：点击主目录下的`exit.bat`文件可以立即终止该服务。  
  ## 项目Build食用方法  
  ①：下载[premake5](https://premake.github.io/)，然后放到项目主文件夹下  
  ②：拉取`nlohmann`官方git代码到项目文件夹中的`include`文件夹下，链接在`include`文件夹下  
  ③：运行项目文件夹下的build.bat文件，构建VS项目。(默认是vs2019版本，其他版本可手动`premake5 vsXXXX`)  
  ④：打开项目，生成文件即可。(暂不支持×64)
  
  
  

