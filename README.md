# MSXMLReader

MSXMLReader は、Microsoft XML Core Services (MSXML) の SAX パーサを使って XML を読み込むためのヘッダオンリーライブラリです。

## 概要

MSXMLReader は、MSXML の SAX パーサをラップし、XML の読み込みを簡単に行えるようにするヘッダオンリーライブラリです。DTD の処理や外部参照の制御をマクロで切り替え可能で、複数のサンプルアプリケーションで動作を確認できます。

## 特徴

- ヘッダオンリーで使用可能
- MSXML の SAX パーサをラップ
- DTD の処理や外部参照の制御をマクロで切り替え可能
- 複数のサンプルアプリケーションで動作確認済み

## 注意事項

- MSXMLReader は Windows 専用です。
- 外部参照を許可する場合は、信頼できる XML と参照先を使用してください。

## ビルド環境

- Windows 11 64bit 以降
- Visual Studio 2026 以降 または MinGW-w64
- MSXML 6.0 がインストールされていること

## サンプルのビルド方法

各 `.cpp` は1つのコンソールアプリです。Visual Studio の x64 Native Tools Command Prompt でこのディレクトリを開き、サンプルごとにビルドしてください。

```bat
cl /nologo /std:c++latest /EHsc 01-default.cpp ole32.lib oleaut32.lib comsuppw.lib
cl /nologo /std:c++latest /EHsc 02-internal-dtd.cpp ole32.lib oleaut32.lib comsuppw.lib
cl /nologo /std:c++latest /EHsc 03-external-references-only.cpp ole32.lib oleaut32.lib comsuppw.lib
cl /nologo /std:c++latest /EHsc 04-external-dtd.cpp ole32.lib oleaut32.lib comsuppw.lib
```

Windows上でCMakeを使う場合は、MSVCまたはMinGW-w64のGCC/Clangツールチェーンを選択できます。`04-external-dtd` は入力引数に `wmain` を使うため、MinGW系ではCMakeが `-municode` を付与します。

|実行コマンド|`MSXML_READER_USE_DTD`|`MSXML_READER_USE_EXTERNAL_REFERENCE`|確認できる動作|
|:---:|:---:|:---:|:---|
|`01-default.exe`|未定義|未定義|DTD のない XML を読み、`Plain XML` を表示|
|`02-internal-dtd.exe`|定義|未定義|内部 DTD のエンティティを展開し、`Internal DTD` を表示|
|`03-external-references-only.exe`|未定義|定義|外部参照を許可していても DTD は禁止され、`DTD rejected` を表示|
|`04-external-dtd.exe external-dtd.xml`|定義|定義|同じディレクトリの DTD が補った属性値 `External DTD` を表示|

`04-external-dtd.exe` に付属の `external-dtd.xml` を渡すと、隣にある `external-dtd.dtd` を読みます。外部参照を許可する構成では、信頼できる XML と参照先を使ってください。

複数の `.cpp` から `MSXMLReader.hpp` を使うアプリでは、2つのマクロの定義有無を全ファイルでそろえてください。プロジェクト共通のプリプロセッサ定義で指定できます。無効にする場合は定義を外します。`=0` でも定義済みとして扱われます。
