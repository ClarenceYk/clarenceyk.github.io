---
title: 使用 SIPp 测试 Asterisk 处理能力
date: 2021-03-05 17:06:27
updated: 2021-03-05 17:06:27
tags:
- 操作方法
- QEMU
- ARM64
- Ubuntu
---

本文将简单介绍 SIPp 的使用方法，然后重点介绍如何使用 SIPp 对 Asterisk 服务进行 SIP 注册与通话测试。

<!-- more -->

## 环境

本文所有操作在 `Ubuntu 20.04` 和 `Debian 10` 操作系统上完成。

## SIPp

SIPp 是一款针对 SIP 协议的开源测试工具，使用此工具可以自定义发送各类 SIP 协议请求以及接收分析 SIP 协议响应，可用于模拟 VoIP 通话进行话音测试。

### 软件安装

下载并解压源码：

```shell
wget -c https://github.com/SIPp/sipp/releases/download/v3.6.1/sipp-3.6.1.tar.gz -O - | tar -xz -C /tmp
```

编译安装：

```shell
cd /tmp/sipp-3.6.1
./build.sh --common
make -j4
sudo make install
```

### 使用方法

*SIPp 的详细使用方法请参考[官方手册](http://sipp.sourceforge.net/doc/reference.html)。此外可使用 `sipp -h` 命令查看 SIPp 软件的命令行参数说明。*

#### 命令行参数

SIPp 使用 XML 描述语言来描述软件交互行为（情景），可通过命令行参数 `-sf` 指定自定义的 XML 文件：

```shell
sipp -sf <your_xml_file>
```

同时 SIPp 也内置了一些 XML 文件来描述一些特定场景的交互过程，可通过 `-sn` 命令行参数加载内置文件：

```shell
sipp -sn <default_xml_file>
```

例如，运行 SIPp 内置的 SIP 服务端示例：

```shell
./sipp -sn uas
```

在同一台电脑上再运行 SIPp 内置的 SIP 客户端示例：

```shell
./sipp -sn uac 127.0.0.1
```

此时 SIPp 便开始执行测试流程，界面如下：

{% img /2021/03/05/testing-asterisk-using-sipp/a.png '"内置 UAC 测试界面" "内置 UAC 测试界面"' %}

我们不但可以传递预设参数给 SIPp，可能传递自定义参数，自定义参数保存在 csv 文件中，可通过如下方式传入：

```shell
sipp -inf <your_csv_file>
```

SIPp 使用的 csv 文件**第一行**指定读取方式：

- `SEQUENTIAL` 顺序读取
- `RANDOM` 随即读取
- `USER` 基于用户读取

从第2行开始便是 SIPp 读取的数据，每个测试实例读取一行，每行中的参数以 `;` 号隔开：

```plain
uklar;age;address
```

在 XML 文件中引用如上参数则分别使用 `[field0]`、`[field1]`、`[field2]`。

#### XML 情景描述

这里用 SIP 注册为例讲解如何编写情景描述文件。

SIPp 使用的 XML 描述文件均以如下内容开头：

```xml
<?xml version="1.0" encoding="ISO-8859-1" ?>
<!DOCTYPE scenario SYSTEM "sipp.dtd">
```

接着便是描述情景的 section：

```xml
<scenario name="My UAC registor">
<!-- describe your scenario here -->
</scenario>
```

所有的交互动作均要编写在 `<scenario/>` section 里。

根据 SIP 协议示例文档（[rfc3665](https://tools.ietf.org/html/rfc3665)）确定交互流程：

{% codeblock 注册交互流程 line_number:false %}
UAC                       SIP Server
 |                            |
 |          REGISTER          |
 |--------------------------->|
 |      401 Unauthorized      |
 |<---------------------------|
 |          REGISTER          |
 |--------------------------->|
 |           200 OK           |
 |<---------------------------|
 |                            |
{% endcodeblock %}

根据交互流程编写基本框架：

```xml
<send retrans="500">
  <![CDATA[

    REGISTER

  ]]>
</send>

<recv response="401" auth="true">
</recv>

<send retrans="500">
  <![CDATA[
  
    REGISTER

  ]]>
</send>

<recv response="200" crlf="true">
</recv>
```

其中 `<![CDATA[ ... ]]>` 用于指定 SIPp 发送的 SIP 协议帧内容，我们可以根据不同的场景自定义帧内容。

例如，模拟客户端 101 向服务器（asterisk） 192.168.199.59 发送 SIP 注册请求：

```xml
<![CDATA[

  REGISTER sip:192.168.199.59 SIP/2.0
  Via: SIP/2.0/[transport] [local_ip]:[local_port];branch=[branch]
  Max-Forwards: 70
  From: "101" <sip:101@192.168.199.59>;tag=[pid]101[call_number]
  To: "101" <sip:101@192.168.199.59>
  Call-ID: [call_id]
  CSeq: 1 REGISTER
  Contact: <sip:101@[local_ip]:[local_port]>;transport=[transport]
  Expires: 3600
  Content-Length: 0

]]>
```

如上，所有 `[]` 内的字段均可通过命令行参数指定，或 SIPp 自动填入默认值。

发送 SIP 注册消息后，我们等待服务端响应 `401` 未授权消息，然后发送携带认证信息的注册请求：

```xml
<![CDATA[

  REGISTER sip:192.168.199.59 SIP/2.0
  Via: SIP/2.0/[transport] [local_ip]:[local_port];branch=[branch]
  Max-Forwards: 70
  From: "101" <sip:101@192.168.199.59>;tag=[pid]101[call_number]
  To: "101" <sip:101@192.168.199.59>
  Call-ID: [call_id]
  CSeq: 2 REGISTER
  Contact: <sip:101@[local_ip]:[local_port]>;transport=[transport]
  [authentication username=101 password=1234abcd]
  Expires: 3600
  Content-Length: 0

]]>
```

我们只需在消息帧中加入 `[authentication username=USERNAME password=PASSWD]`，SIPp 便可根据前一条消息的内容自动计算并填入认证信息。

需要注意的是，定义前一条消息时（这里是 `401`），需要加入属性 `auth="true"`。

最后我们等待服务端发送 `200 OK` 响应以完成 SIP 注册流程。

#### 批量运行

SIPp 的主要功能是将定义的情景通过不同的参数实例化出多个测试进程，实现并发测试。

还是以 SIP 注册为例，我们将之前的单用户注册改造为多用户并发向指定服务器注册，命令行指令如下：

```shell
sipp -sf ./register.xml <server_ip> -i <local_ip> -users <number_of_users> -inf ./data.csv
```

相应参数的解释如下：

- `server_ip`：指定服务端 IP 地址，在 XML 文件中通过 `[remote_ip]` 引用
- `local_ip`：指定客户端 IP 地址，在 XML 文件中通过 `[local_ip]` 引用
- `number_of_users`：指定同时存在的用户数量

假定我们需要同时存在 3 个用户向服务器发起注册，则：

`data.csv` 文件内容如下：

```plaintext
SEQUENTIAL
101,[authentication username=101 password=1234abcd]
102,[authentication username=102 password=1234abcd]
103,[authentication username=103 password=1234abcd]
```

再修改前面例子中部分参数：

- 服务端 IP 地址 `192.168.199.59` 替换为 `[remote_ip]`
- 用户名 `101` 替换为 `[field0]`
- 授权信息 `[authentication username=101 password=1234abcd]` 替换为 `[field1]`

启动批量注册测试：

```shell
sipp -sf ./register.xml 192.168.199.59 -i 192.168.199.30 -users 3 -inf ./data.csv
```

## 通话测试

以下操作流程中涉及到的文件都能[点击此处](https://gist.github.com/ClarenceYk/9c67e4c7d4d194b32c7a34a95209159a)获取。

模拟 SIP 通话场景如下图：

{% codeblock 模拟场景 line_number:false %}
+---------------------------+    +---------------------------+     +----------------------------+
| Host Caller               |    | Asterisk Server           |     | Host Callee                |
|                           |    |                           +<--->+                            |
|   N_Users: 49             +<-->+   N_Users: 98             |     |   N_Users: 48              |
|   Codec  : G729           |    |   Codecs : G711a G729     +<-+  |   Codec  : G711a           |
|   IP     : 192.168.199.30 |    |   IP     : 192.168.199.59 |  |  |   IP     : 192.168.199.227 |
+---------------------------+    +---------------------------+  |  +----------------------------+
                                                                |
                                                                |  +----------------------------+
                                                                |  | Yealink                    |
                                                                |  |                            |
                                                                +->+   Codec  : G711a           |
                                                                   |   IP     : 192.168.199.35  |
                                                                   +----------------------------+
{% endcodeblock %}

在 Asterisk 服务器上创建 **98** 个用户：

```shell
for i in {101..198}; do ext_manage.sh add $i; done
asterisk -rx "core stop now"
asterisk
```

在 `Host Caller` 上生成 `test_data.csv` 文件：

```shell
echo "SEQUENTIAL" > test_data.csv
for i in {101..149}; do echo "$i;$(expr $i + 49);[authentication username=$i password=1234abcd];[authentication username=$(expr $i + 49) password=1234abcd]" >> test_data.csv; done
```

在 `Host Callee` 上生成 `test_data.csv` 文件：

```shell
echo "SEQUENTIAL" > test_data.csv
for i in {101..148}; do echo "$i;$(expr $i + 49);[authentication username=$i password=1234abcd];[authentication username=$(expr $i + 49) password=1234abcd]" >> test_data.csv; done
```

在 `Host Caller` 上用 SIPp 注册 **49** 个用户：

```shell
sudo sipp -sf ./my_caller_register.xml 192.168.199.59 -i 192.168.199.30 -users 49 -inf ./test_data.csv
```

在 `Host Callee` 上用 SIPp 注册 **48** 个用户：

```shell
sudo sipp -sf ./my_callee_register.xml 192.168.199.59 -i 192.168.199.227 -users 48 -inf ./test_data.csv
```

在 `Yealink` 上使用账户 `198` 向服务器注册，并在设置中关闭视频通话，只开启 g711a 话音编码。

在 `Host Callee` 上用 SIPp 启动接收 **48** 路通话：

```shell
sudo sipp -sf ./my_callee_flow.xml 192.168.199.59 -i 192.168.199.227 -users 48 -inf ./test_data.csv
```

在 `Host Caller` 上用 SIPp 启动呼叫 **49** 路通话：

```shell
sudo sipp -sf ./my_caller_flow.xml 192.168.199.59 -i 192.168.199.30 -users 49 -inf ./test_data.csv
```

此时我们可以在 `Yealink` 上收到呼叫，接听后可通过话音质量来主观感受服务器压力状态。

同时可查看服务器 CPU 占用状况：

{% img /2021/03/05/testing-asterisk-using-sipp/b.png '"CPU 占用状态" "CPU 占用状态"' %}
