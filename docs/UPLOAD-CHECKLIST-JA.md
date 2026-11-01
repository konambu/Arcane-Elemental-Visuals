# 投稿前の確認 — 0.1.0-beta1

このフォルダはローカルの投稿準備一式です。Steamにはまだ登録していません。

- `Arcane Elemental Visuals/`：FCSで投稿する配布用MOD。DLL2本、素材、GPL本文、対応ソースを同梱。
- `Arcane-Elemental-Visuals-0.1.0-beta1.zip`：同じMODの手動導入・保管用ZIP。FCSには展開済みフォルダを使う。
- `preview.jpg`：512×512 JPEG、約22KB。実際の素材を単独描画したプレビュー。実ゲームのスクリーンショットではない。
- `WORKSHOP-DESCRIPTION-JA.txt` / `-EN.txt`：Steamに貼り付ける説明文。
- `PACKAGE-MANIFEST.json`：配布ファイルのサイズ・SHA256。

## 済んだこと

- [x] 火球v0.23安定版の対応ソースを復元。
- [x] 2本のDLLをソースから再生成し、使用中のDLLとSHA256まで一致。
- [x] PC固有パスを除いたビルド手順とimport lib生成を用意。
- [x] 火球・土・地形・岩・竜巻・ミサイル炎の6系統の隔離検査。
- [x] GPLv3本文、対応ソース、従来のMIT表示、素材別クレジットを同梱。
- [x] 旧DLL・ログ・バックアップ・テストDLL・個人の設定とセーブを除外。
- [x] FCSの変更レコードと依存関係を保ったまま、説明・作者表示・版だけを公開用に更新。
- [x] 日英の説明文と、JPEGのサムネイルを用意。

## 残る実機確認

- [x] v4の横向きのミサイル炎の見た目をプレイヤーがKenshi内で確認済み（2026-11-01）。
- [ ] 原作と当MOD・必須環境だけの構成で3魔法を使えるか。
- [ ] エリア移動・セーブ読込・再起動後にも正常に動くか。
- [ ] 非公開のWorkshopアイテムから購読導入し、番号フォルダの素材とDLLが読めるか。

ローカル版があるままでは、実際にはローカル版が読み込まれて成功する場合がある。
Workshop確認時はゲームを完全終了し、既存のローカル版を安全な場所へ退避してから行う。
公開用コピーと作業中のコピーを同時に有効にしない。

## Steamでの手順

1. 実機確認後、ゲームを完全終了して既存MODフォルダをバックアップし、配布用コピーを `Kenshi/mods/Arcane Elemental Visuals/` に配置。
2. 原作の編集用コピーが必要な場合は `mods/ArcaneMagic/ArcaneMagic.mod` の形で用意。原作を編集対象にしない。
3. SteamのKenshi「プレイ」→「Launch Game Editor」。当アドオンをActiveにして開き、依存・差分を確認して保存。
4. 「Steam Workshop」でタイトル `Arcane Elemental Visuals`、版 `0.1.0-beta1`、作者表示、説明とJPEGを設定。新規投稿としてIDを発行する。
5. 非公開の状態で確認。初回に求められるWorkshop利用規約は投稿者本人が確認・同意する。
6. 管理画面の「必要なアイテム」にArcane Magic（ID `3784221846`）を追加。RE_Kenshiは外部リンクからの別途導入であることを残す。
7. 購読導入の確認後に一般公開へ変更。同じWorkshop IDを維持して更新する。

FCSが生成する `.info` はWorkshop IDを保持するため、初回投稿後にバックアップする。
現在の作者表示は仮の `Arcane Elemental Visuals contributors`。投稿時に好みの表示へ変更できる。
公開ページの作者アカウントは、実際に投稿するSteamアカウントになる。

根拠：[Kenshi投稿ガイド](https://steamcommunity.com/sharedfiles/filedetails/?id=708280472)、
[元MODの許可・必須環境](https://steamcommunity.com/sharedfiles/filedetails/?id=3784221846)、
[KenshiLibのライセンス方針](https://github.com/BFrizzleFoShizzle/KenshiLib)、
[Valveの初回規約説明](https://partner.steamgames.com/doc/features/workshop/implementation#WorkshopLegalAgreement)。
