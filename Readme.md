这个是EVEngine的第三方库，可能有部分代码是被修改过的，在升级时要注意

master分支存放源代码
linux, darwin, win32, android, ios 等不同的二进制版本发布到github上供下载

在CMake中使用第三方下载安装的指令，将代码下载下来编译，并安装到third-party/binary下


推荐在不同平台下编译，然后将对应二进制库存放到github release附件中使用，在CMake中可选是使用预编译的版本还是下载源代码编译

每个third-party的release版，要打tag，方便主仓库用命令下载之前版本或者回滚测试。
不使用submodule因为这个第三方库列表可能会频繁修改，打补丁或升降级，普通的引擎开发者只需要关注核心代码，无需考虑修改第三方库