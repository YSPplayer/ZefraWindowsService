--这个是我们VS解决方案的名称
workspace "ZefraWindowsService"
configurations { "Debug", "Release" }	-- 解决方案配置项，Debug和Release默认配置
platforms { "x86", "x64" }--配置解决方案平台
location "build"	--解决方案文件夹	

-- project：对应vs中的项目
project "ZefraServiceToos"
kind "SharedLib"--动态库文件
language "C++"
targetdir "bin/%{cfg.buildcfg}"
files { "ZefraServiceToos/*.h", "ZefraServiceToos/*.cpp" }


project "ZefraBgSwitchExe" --这个是我们启动服务的主程序
kind "ConsoleApp" -- 项目类型：控制台程序
language "C++"    --语言类型
targetdir "bin/%{cfg.buildcfg}"--指定文件的输出路径，cfg.buildcfg对应上面的Debug和Release
--*表示匹配任意字符的文件,**表示匹配下面任意级目录的字符文件
files { "./ZefraBgSwitchExe/*.cpp" }	 -- 指定加载哪些文件或哪些类型的文件
links { "ZefraServiceToos" } --这个表示的是自动对ZefraServiceToos库文件添加依赖
includedirs {"include","ZefraServiceToos" }-- C/C++包含附加库目录

project "ZefraBgSwitchService"
  kind "ConsoleApp"
  language "C++"
  targetdir "bin/%{cfg.buildcfg}"
  files { "./ZefraBgSwitchService/*.h", "./ZefraBgSwitchService/*.cpp" }
  includedirs { "ZefraServiceToos" }
  links { "ZefraServiceToos" }


  -- Debug配置项属性
filter "configurations:Debug"
    defines { "_DEBUG" }			-- 定义DEBUG宏（这可以算是默认配置）
    symbols "On"				-- 开启调试符号，
    
-- Release配置项属性
filter "configurations:Release"
    defines { "NDEBUG" }		-- 定义NDEBUG宏（这可以算是默认配置）
    optimize "On"				-- 开启优化参数
