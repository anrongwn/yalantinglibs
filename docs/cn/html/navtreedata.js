/*
 @licstart  The following is the entire license notice for the JavaScript code in this file.

 The MIT License (MIT)

 Copyright (C) 1997-2020 by Dimitri van Heesch

 Permission is hereby granted, free of charge, to any person obtaining a copy of this software
 and associated documentation files (the "Software"), to deal in the Software without restriction,
 including without limitation the rights to use, copy, modify, merge, publish, distribute,
 sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is
 furnished to do so, subject to the following conditions:

 The above copyright notice and this permission notice shall be included in all copies or
 substantial portions of the Software.

 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING
 BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,
 DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

 @licend  The above is the entire license notice for the JavaScript code in this file
*/
var NAVTREE =
[
  [ "yaLanTingLibs", "index.html", [
    [ "struct_pack简介", "md_src_struct_pack_doc__introduction__c_n.html", [
      [ "序列化", "md_src_struct_pack_doc__introduction__c_n.html#autotoc_md1", [
        [ "基本用法", "md_src_struct_pack_doc__introduction__c_n.html#autotoc_md2", null ],
        [ "指定序列化返回的容器类型", "md_src_struct_pack_doc__introduction__c_n.html#autotoc_md3", null ],
        [ "将序列化结果保存到已有的容器尾部", "md_src_struct_pack_doc__introduction__c_n.html#autotoc_md4", null ],
        [ "将序列化结果保存到指针指向的内存中。", "md_src_struct_pack_doc__introduction__c_n.html#autotoc_md5", null ],
        [ "多参数序列化", "md_src_struct_pack_doc__introduction__c_n.html#autotoc_md6", null ]
      ] ],
      [ "反序列化", "md_src_struct_pack_doc__introduction__c_n.html#autotoc_md7", [
        [ "基本用法", "md_src_struct_pack_doc__introduction__c_n.html#autotoc_md8", null ],
        [ "从指针指向的内存中反序列化", "md_src_struct_pack_doc__introduction__c_n.html#autotoc_md9", null ],
        [ "反序列化（将结果保存到已有的对象中）", "md_src_struct_pack_doc__introduction__c_n.html#autotoc_md10", null ],
        [ "多参数反序列化", "md_src_struct_pack_doc__introduction__c_n.html#autotoc_md11", null ],
        [ "部分反序列化", "md_src_struct_pack_doc__introduction__c_n.html#autotoc_md12", null ]
      ] ],
      [ "支持序列化所有的STL容器、自定义容器和optional", "md_src_struct_pack_doc__introduction__c_n.html#autotoc_md13", null ],
      [ "benchmark", "md_src_struct_pack_doc__introduction__c_n.html#autotoc_md14", [
        [ "测试方法", "md_src_struct_pack_doc__introduction__c_n.html#autotoc_md15", null ],
        [ "测试对象", "md_src_struct_pack_doc__introduction__c_n.html#autotoc_md16", null ],
        [ "测试环境", "md_src_struct_pack_doc__introduction__c_n.html#autotoc_md17", null ],
        [ "测试结果", "md_src_struct_pack_doc__introduction__c_n.html#autotoc_md18", null ]
      ] ],
      [ "向前/向后兼容性", "md_src_struct_pack_doc__introduction__c_n.html#autotoc_md19", null ],
      [ "为什么struct_pack更快？", "md_src_struct_pack_doc__introduction__c_n.html#autotoc_md20", null ],
      [ "附录", "md_src_struct_pack_doc__introduction__c_n.html#autotoc_md21", [
        [ "测试代码", "md_src_struct_pack_doc__introduction__c_n.html#autotoc_md22", null ]
      ] ]
    ] ],
    [ "struct_pack Introduction", "md_src_struct_pack_doc__introduction_en.html", [
      [ "Serialization", "md_src_struct_pack_doc__introduction_en.html#autotoc_md24", [
        [ "Basic Usage", "md_src_struct_pack_doc__introduction_en.html#autotoc_md25", null ],
        [ "Explicit data container", "md_src_struct_pack_doc__introduction_en.html#autotoc_md26", null ],
        [ "Append the result at the end of existing data container", "md_src_struct_pack_doc__introduction_en.html#autotoc_md27", null ],
        [ "Save the results to memory location indicated by pointer", "md_src_struct_pack_doc__introduction_en.html#autotoc_md28", null ],
        [ "Multi-parameter serialization", "md_src_struct_pack_doc__introduction_en.html#autotoc_md29", null ]
      ] ],
      [ "Deserialization", "md_src_struct_pack_doc__introduction_en.html#autotoc_md30", [
        [ "Basic Usage", "md_src_struct_pack_doc__introduction_en.html#autotoc_md31", null ],
        [ "deserialize from pointers", "md_src_struct_pack_doc__introduction_en.html#autotoc_md32", null ],
        [ "deserialize to an existing object", "md_src_struct_pack_doc__introduction_en.html#autotoc_md33", null ],
        [ "Multi-parameter deserialization", "md_src_struct_pack_doc__introduction_en.html#autotoc_md34", null ],
        [ "Partial deserialization", "md_src_struct_pack_doc__introduction_en.html#autotoc_md35", null ]
      ] ],
      [ "support std containers, std::optional and custom containers", "md_src_struct_pack_doc__introduction_en.html#autotoc_md36", null ],
      [ "benchmark", "md_src_struct_pack_doc__introduction_en.html#autotoc_md37", [
        [ "Test case", "md_src_struct_pack_doc__introduction_en.html#autotoc_md38", null ],
        [ "Test objects", "md_src_struct_pack_doc__introduction_en.html#autotoc_md39", null ],
        [ "Test Environment", "md_src_struct_pack_doc__introduction_en.html#autotoc_md40", null ],
        [ "Test results", "md_src_struct_pack_doc__introduction_en.html#autotoc_md41", null ]
      ] ],
      [ "Forward/backward compatibility", "md_src_struct_pack_doc__introduction_en.html#autotoc_md42", null ],
      [ "Why is struct_pack faster?", "md_src_struct_pack_doc__introduction_en.html#autotoc_md43", null ],
      [ "Appendix", "md_src_struct_pack_doc__introduction_en.html#autotoc_md44", [
        [ "Test code", "md_src_struct_pack_doc__introduction_en.html#autotoc_md45", null ]
      ] ]
    ] ],
    [ "coro_rpc简介", "md_src_coro_rpc_doc_coro_rpc_introduction_cn.html", [
      [ "coro_rpc的易用性", "md_src_coro_rpc_doc_coro_rpc_introduction_cn.html#autotoc_md47", [
        [ "rpc_server端", "md_src_coro_rpc_doc_coro_rpc_introduction_cn.html#autotoc_md48", null ],
        [ "rpc函数支持任意参数", "md_src_coro_rpc_doc_coro_rpc_introduction_cn.html#autotoc_md49", null ]
      ] ],
      [ "和grpc、brpc比较易用性", "md_src_coro_rpc_doc_coro_rpc_introduction_cn.html#autotoc_md50", [
        [ "rpc易用性比较", "md_src_coro_rpc_doc_coro_rpc_introduction_cn.html#autotoc_md51", null ],
        [ "异步编程模型比较", "md_src_coro_rpc_doc_coro_rpc_introduction_cn.html#autotoc_md52", null ]
      ] ],
      [ "coro_rpc更多特色", "md_src_coro_rpc_doc_coro_rpc_introduction_cn.html#autotoc_md53", [
        [ "同时支持实时任务和延时任务", "md_src_coro_rpc_doc_coro_rpc_introduction_cn.html#autotoc_md54", null ],
        [ "服务端同时支持协程和异步回调", "md_src_coro_rpc_doc_coro_rpc_introduction_cn.html#autotoc_md55", null ]
      ] ],
      [ "benchmark", "md_src_coro_rpc_doc_coro_rpc_introduction_cn.html#autotoc_md56", [
        [ "测试环境", "md_src_coro_rpc_doc_coro_rpc_introduction_cn.html#autotoc_md57", null ],
        [ "测试case", "md_src_coro_rpc_doc_coro_rpc_introduction_cn.html#autotoc_md58", [
          [ "极限qps测试", "md_src_coro_rpc_doc_coro_rpc_introduction_cn.html#autotoc_md59", null ],
          [ "ping-pong测试", "md_src_coro_rpc_doc_coro_rpc_introduction_cn.html#autotoc_md60", null ],
          [ "长尾测试", "md_src_coro_rpc_doc_coro_rpc_introduction_cn.html#autotoc_md61", null ]
        ] ],
        [ "benchmark备注", "md_src_coro_rpc_doc_coro_rpc_introduction_cn.html#autotoc_md62", null ]
      ] ],
      [ "使用约束", "md_src_coro_rpc_doc_coro_rpc_introduction_cn.html#autotoc_md63", null ]
    ] ],
    [ "Introduction", "md_src_coro_rpc_doc_coro_rpc_introduction_en.html", [
      [ "Usability", "md_src_coro_rpc_doc_coro_rpc_introduction_en.html#autotoc_md65", [
        [ "server", "md_src_coro_rpc_doc_coro_rpc_introduction_en.html#autotoc_md66", null ],
        [ "RPC with any parameters", "md_src_coro_rpc_doc_coro_rpc_introduction_en.html#autotoc_md67", null ]
      ] ],
      [ "Compare with grpc/brpc", "md_src_coro_rpc_doc_coro_rpc_introduction_en.html#autotoc_md68", [
        [ "Usability", "md_src_coro_rpc_doc_coro_rpc_introduction_en.html#autotoc_md69", null ],
        [ "Asynchronous Model", "md_src_coro_rpc_doc_coro_rpc_introduction_en.html#autotoc_md70", null ]
      ] ],
      [ "More features", "md_src_coro_rpc_doc_coro_rpc_introduction_en.html#autotoc_md71", [
        [ "Real-time Tasks and Non-Real-time Tasks", "md_src_coro_rpc_doc_coro_rpc_introduction_en.html#autotoc_md72", null ],
        [ "Asynchronous mode", "md_src_coro_rpc_doc_coro_rpc_introduction_en.html#autotoc_md73", null ]
      ] ],
      [ "Benchmark", "md_src_coro_rpc_doc_coro_rpc_introduction_en.html#autotoc_md74", [
        [ "System Configuration", "md_src_coro_rpc_doc_coro_rpc_introduction_en.html#autotoc_md75", null ],
        [ "Test case", "md_src_coro_rpc_doc_coro_rpc_introduction_en.html#autotoc_md76", [
          [ "Peak QPS test", "md_src_coro_rpc_doc_coro_rpc_introduction_en.html#autotoc_md77", null ],
          [ "ping-pong test", "md_src_coro_rpc_doc_coro_rpc_introduction_en.html#autotoc_md78", null ],
          [ "long-tail test", "md_src_coro_rpc_doc_coro_rpc_introduction_en.html#autotoc_md79", null ]
        ] ],
        [ "Notes on benchmark test", "md_src_coro_rpc_doc_coro_rpc_introduction_en.html#autotoc_md80", null ]
      ] ],
      [ "Known Limitations", "md_src_coro_rpc_doc_coro_rpc_introduction_en.html#autotoc_md81", null ]
    ] ],
    [ "模块", "modules.html", "modules" ],
    [ "类", "annotated.html", [
      [ "类列表", "annotated.html", "annotated_dup" ],
      [ "类索引", "classes.html", null ],
      [ "类成员", "functions.html", [
        [ "全部", "functions.html", null ],
        [ "函数", "functions_func.html", null ]
      ] ]
    ] ],
    [ "文件", "files.html", [
      [ "文件列表", "files.html", "files_dup" ]
    ] ]
  ] ]
];

var NAVTREEINDEX =
[
"annotated.html"
];

var SYNCONMSG = '点击 关闭 面板同步';
var SYNCOFFMSG = '点击 开启 面板同步';