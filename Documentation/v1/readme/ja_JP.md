## ドキュメント
[English Documentation](Documentation/v1/docs/en_US.md) \
[Documentação em Português](Documentation/v1/docs/pt_BR.md) \
[日本語ドキュメント](Documentation/v1/docs/ja_JP.md)

## その他の言語
[Português](Documentation/v1/readme/pt_BR.md) \
[English](Documentation/v1/readme/en_US.md)

## はじめに
SAH (Stack as Heap)は、仮想メモリのプリミティブ上に構築された、スタック型アロケータを提供する軽量な低レイヤーC言語ライブラリです。一時的なメモリ割り当て、アリーナ、パーサー、ランタイム、仮想マシンなど、予測可能で高速なメモリ管理が求められるパフォーマンス重視のコード向けに設計されています。

本ライブラリは、汎用的なヒープアロケータを代替するものではありません。

## 機能
SAHは固定サイズのメモリ領域をマップし、スタックの下方にガードページを配置することで、OSレベルでオーバーフローを検知します。メモリの割り当て（Allocation）および解放（Deallocation）は $O(1)$ であり、分岐（Branch）を含みません。スタックは実際のCPUセマンティクスに従って下方伸長（Grow downward）し、明示的なベースポインタ（BP）とスタックポインタ（SP）によって管理されます。

ライブラリ側でのオーバーフロー検知は行いません。スタックポインタがガードページに到達した場合、OSがセグメンテーション違反（Segmentation Fault）を発生させます。これは、バグを早期に発見（Fail-fast）させ、アロケータのブランチフリーな特性を維持するための設計上の意図によるものです。

2つのインターフェースを提供しています：

1. Rawインターフェース: 呼び出し側が手動でサイズを管理します。
2. Structuredインターフェース: メタデータを保存するため、呼び出し側でサイズを追跡する必要がありません。

## レイアウト
SAHは2つの連続した領域をマップします。1つ目は割り当て領域の底部に配置される「ガードページ」で、アクセス権限を持ちません。2つ目は、デフォルトで4096バイトの「利用可能なスタック領域」です。スタックポインタは利用可能領域の最上部から始まり、割り当てごとに下方へ移動します。
```
High address  [ Stack region  - 4096 bytes ]  <-- BP, SP starts here
              [ Guard page   - 1 page      ]  <-- PROT_NONE / PAGE_NOACCESS
Low address
```

## 目標
- ホットパスにおける境界チェック（Bounds check）を排除し、オーバーヘッドを最小化
- 実際のスタックセマンティクス（下方伸長、明示的なBPおよびSP）の採用
- ガードページによるOSレベルのオーバーフロー検知
- 最小限のAPIによる、シンプルで予測可能な動作
- 一時的な割り当てやアリーナ型の使用パターンに最適化

## 使用例
SAHを使用するには、ヘッダーをインクルードする前に、ただ1つの翻訳単位（Translation unit）で `SAH_IMPLEMENTATION` を定義してください。
```c
#define SAH_IMPLEMENTATION
#include "sah.h"

int main(void)
{
    struct sah_stack* s = screate();

    /* raw push/pop — caller tracks size */
    int* x = push(s, sizeof(int));
    *x = 123;

    /* structured push/pop — size stored internally */
    char* buf = spush(s, 32);
    /* use buf ... */
    spop(s);              /* frees buf  */
    pop(s, sizeof(int)); /* frees x    */

    sdestroy(s);
    return 0;
}
```

## サポートされているプラットフォーム (Platform Support)
| プラットフォーム | ステータス |
|-----------------|-----------|
| Linux           | 対応済み   |
| Windows         | 対応済み   |

Linux環境では `mmap` および `mprotect` を使用します。Windows環境では `VirtualAlloc` および `VirtualProtect` を使用します。APIは両プラットフォームで共通です。

## ライセンス (License)
BSD 3-Clause

## 備考 (Observation)
SAHは学習および趣味を目的とした個人プロジェクトです。本プロジェクトは実験的なものであり、そのように取り扱われるべきです。プロダクション環境での使用は、自己責任でお願いいたします。
