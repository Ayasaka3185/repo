# UE5_版本控制 协作说明



本文档旨在对版本控制进行简单说明, 包含从零开始下载所需软件、克隆项目提交自己的修改、发起合并，以及避免覆盖别人文件
。

仓库地址：

```text
https://github.com/Ayasaka3185/repo.git
```

---

## 1. Git, GitHub 和 Git LFS

### Git

Git 是本地版本控制工具。  
它负责记录电脑里项目文件的变化，比如：

- 今天改了哪个 UI
- 谁改了哪个蓝图
- 哪个版本可以正常运行
- 如果改坏了，能回到之前的版本

Git 是装在电脑上的工具。

### GitHub

GitHub 是线上仓库。  
可以把它理解成“项目云盘 + 修改记录 + 审批系统”。

大家不会直接把文件互相发来发去，而是：

1. 从 GitHub 下载项目。
2. 在自己电脑上修改。
3. 提交到自己的分支。
4. 发 Pull Request。
5. 通过检查后再合并到主分支。

### Git LFS

UE5 项目有很多大文件，比如：

- `.uasset`
- `.umap`
- `.fbx`
- `.wav`
- `.mp4`
- `.png`
- `.psd`

普通 Git 不适合直接管理这些大文件，所以要使用 Git LFS。  


本项目已经配置了 Git LFS，但每个人电脑上也必须安装并启用它。

---

## 2. 第一次使用前要安装什么

### 2.1 安装 Git for Windows

下载地址：

```text
https://git-scm.com/download/win
```

安装时基本一路默认即可。  


安装完成后，打开 PowerShell，输入：

```powershell
git --version
```

如果能看到类似下面的内容，说明 Git 安装成功：

```text
git version 2.54.0.windows.1
```

### 2.2 安装 Git LFS

下载地址：

```text
https://git-lfs.com/
```

安装完成后，打开 PowerShell，输入：

```powershell
git lfs version
```

如果能看到类似下面的内容，说明 Git LFS 安装成功：

```text
git-lfs/3.7.1
```

然后再输入：

```powershell
git lfs install
```

看到类似下面的信息即可：

```text
Git LFS initialized.
```

### 2.3 安装 GitHub Desktop

下载地址：

```text
https://desktop.github.com/
```

建议不熟悉命令行的同学优先使用 GitHub Desktop。  
它可以用图形界面完成大部分操作，比如：

- 克隆仓库
- 切换分支
- 查看修改
- 提交 commit
- push 到 GitHub
- 创建 Pull Request

---

## 3. 第一次下载项目

### 3.1 用 GitHub Desktop 下载

1. 打开 GitHub Desktop。
2. 登录自己的 GitHub 账号。
3. 点击 `File`。
4. 点击 `Clone repository`。
5. 选择本项目仓库。
6. 选择本地保存位置，例如：

```text
D:\repo
```

7. 点击 `Clone`。

下载完成后，项目会出现在你选择的文件夹里。

### 3.2 第一次下载后必须拉取 LFS 文件

打开 PowerShell，进入项目文件夹：

```powershell
cd D:\repo
```

然后输入：

```powershell
git lfs pull
```

这一步会把 UE5 的大文件真正下载下来。  
如果不做这一步，有些贴图、音频、蓝图、地图可能会打不开或显示异常。

---

## 4. 分支(branch)是什么

分支可以理解成“每个人自己的工作区”。

不要所有人都直接在主分支上改。  
正确方式是：

1. 主分支保存稳定版本。
2. 每个人新建自己的功能分支。
3. 在自己的分支上修改。
4. 修改完成后发 Pull Request。
5. 检查通过后再合并。

这样即使某个人改坏了，也不会直接影响所有人。

---

## 5. 推荐分支命名规则

分支名请使用英文小写、数字和短横线。  
不要使用中文、空格或特殊符号。

### UI 相关

```text
feature/ui-menu
feature/ui-pause-menu
feature/ui-save-system
```

### 程序功能

```text
feature/save-system
feature/loading-screen
feature/player-controller
```

### 修 Bug

```text
fix/menu-hover-sound
fix/loading-image
fix/pause-menu-position
```

### 美术资源

```text
art/main-menu-background
art/character-portrait
art/loading-screen-image
```

### 音频资源

```text
audio/ui-click-sound
audio/menu-hover-sound
audio/background-music
```

### 测试

```text
test/ui-demo-map
test/save-load-flow
```

---

## 6. git大致工作流程

每天打开项目之前，先同步最新版本。

### GitHub Desktop 操作

1. 打开 GitHub Desktop。
2. 确认当前仓库是本项目。
3. 确认当前分支是你自己的分支。
4. 点击 `Fetch origin`。
5. 如果按钮变成 `Pull origin`，继续点击 `Pull origin`。

### 命令行操作

```powershell
cd D:\repo\repo
git status
git pull
git lfs pull
```

如果你不熟悉命令行，用 GitHub Desktop 即可。

---

## 7. 修改文件后怎么提交

### 7.1 先检查自己改了什么

在 GitHub Desktop 左侧会看到 changed files。  
提交前请认真看一眼，确认没有把不该提交的文件放进去。

### 7.2 写 commit message

commit message 要简单说明你做了什么。

推荐格式：

```text
Add pause menu UI
Fix confirm click sound
Update loading screen image
Add save game mockup
```

### 7.3 提交

GitHub Desktop：

1. 左下角 `Summary` 写提交说明。
2. 点击 `Commit to 当前分支`。
3. 点击 `Push origin`。

命令行：

```powershell
git add .
git commit -m "Add pause menu UI"
git push
```

---

## 8. Pull Request 是什么

Pull Request，简称 PR。  
它的意思是：“我改好了，请把我的分支合并进主分支。”

PR 不是麻烦别人，而是为了保护项目。

PR 可以让大家检查：

- 会不会覆盖别人文件
- UE5 项目能不能打开
- 有没有忘记提交资源
- 有没有提交临时文件
- UI、关卡、音频是否符合当前版本

---

## 9. 怎么创建 Pull Request

### GitHub Desktop

1. 提交 commit。
2. 点击 `Push origin`。
3. 点击 `Create Pull Request`。
4. 浏览器会打开 GitHub 页面。
5. 标题写清楚你做了什么。
6. 描述里写：

```text
本次修改：
- 添加了什么
- 修改了什么
- 需要别人检查什么

测试情况：
- 是否打开过 UE5
- 是否 Play 过
- 是否有已知问题
```

7. 点击 `Create pull request`。

### GitHub 网页

进入仓库页面后，如果 GitHub 提示 `Compare & pull request`，点击即可。

---

## 10. 不要直接改主分支

团队协作时，主分支应该是稳定版本。  
大家不要直接 push 到主分支。

推荐规则：

- `main`：稳定版本，只接受 PR 合并。
- `feature/...`：新功能。
- `fix/...`：修复。
- `art/...`：美术资源。
- `audio/...`：音频资源。
- `test/...`：测试内容。

如果你发现自己在 `main` 上，先不要提交，先新建分支。

GitHub Desktop：

1. 点击顶部 Current branch。
2. 点击 New branch。
3. 输入分支名，例如：

```text
feature/my-work
```

4. 点击 Create branch。

---

## 11. UE5 项目里哪些文件可以提交

通常可以提交：

```text
Config/
Content/
Source/
*.uproject
README.md
.gitignore
.gitattributes
```

美术同学通常会提交：

```text
Content/**/*.uasset
Content/**/*.umap
Content/**/*.png
Content/**/*.jpg
Content/**/*.psd
Content/**/*.fbx
```

音频同学通常会提交：

```text
Content/**/*.wav
Content/**/*.mp3
Content/**/*.ogg
```

程序同学通常会提交：

```text
Source/**/*.h
Source/**/*.cpp
Config/*.ini
```

---

## 12. UE5 项目里哪些文件不要提交

不要提交这些自动生成文件夹：

```text
Binaries/
DerivedDataCache/
Intermediate/
Saved/
```

不要提交这些本地文件：

```text
*.sln
*.VC.db
*.suo
```

原因：

- 它们每个人电脑都会自动生成。
- 文件很大。
- 容易造成冲突。
- 提交后会让仓库变乱。

本项目的 `.gitignore` 已经尽量帮大家过滤这些内容。  
但提交前还是要看一眼 changed files。

---

## 13. UE5 文件冲突要特别小心

UE5 的 `.uasset` 和 `.umap` 是二进制文件。  
它们不像代码一样容易自动合并。

也就是说，如果两个人同时改同一个地图或同一个 Widget 蓝图，可能会冲突。

### 避免冲突的方法

1. 改大文件前，在群里说一声。
2. 不要两个人同时改同一个 `.umap`。
3. 不要两个人同时改同一个 Widget 蓝图。
4. 做大改动时，新建自己的分支。
5. 提交前先 pull 最新版本。
6. 如果 GitHub Desktop 提示冲突，不要乱点，先问负责 Git 的同学。

---

## 14. 美术组提交资源建议

美术资源请尽量放在清晰的目录里，例如：

```text
Content/Art/
Content/Art/UI/
Content/Art/Characters/
Content/Art/Environment/
```

文件命名建议：

```text
T_Loading_Background_01.uasset
T_Menu_Background_01.uasset
SM_Boat_01.uasset
M_Water_Dark_01.uasset
```

建议规则：

- `T_` 表示 Texture
- `M_` 表示 Material
- `MI_` 表示 Material Instance
- `SM_` 表示 Static Mesh
- `SK_` 表示 Skeletal Mesh
- `WBP_` 表示 Widget Blueprint

不要使用：

```text
新建文件
最终版
最终最终版
aaaa
test
```

---

## 15. 音频组提交资源建议

音频资源建议放在：

```text
Content/Audio/
Content/Audio/UI/
Content/Audio/BGM/
Content/Audio/SFX/
```

命名示例：

```text
S_UI_Hover_01.wav
S_UI_ClickConfirm_01.wav
BGM_MainMenu_01.wav
S_SaveComplete_01.wav
```

建议规则：

- `S_` 表示 Sound
- `BGM_` 表示背景音乐
- UI 音效放进 `Audio/UI`
- 环境音放进 `Audio/SFX`

---

## 16. 关卡组提交建议

地图文件 `.umap` 最容易冲突。  
如果多人都要改关卡，请先分工。

建议目录：

```text
Content/Maps/
Content/Maps/UI_Demo/
Content/Maps/Gameplay/
```

命名示例：

```text
L_UI_Demo.umap
L_Gameplay_Test.umap
L_MainMenu.umap
```

改地图前最好在群里说：

```text
我现在要改 L_Gameplay_Test，预计 30 分钟。
```

改完后及时提交并 push。

---

## 17. LFS 使用注意事项

本项目的大资源已经交给 Git LFS 管理。

如果你看到文件很小，但是打开不了，可能是 LFS 文件没有拉下来。  
进入项目目录后运行：

```powershell
git lfs pull
```

如果你刚 clone 项目后 UE5 里资源丢失，也先运行：

```powershell
git lfs pull
```

不要手动删除 `.gitattributes`。  
这个文件决定哪些资源走 LFS。

---

## 18. 常见工作流程

### 18.1 我今天要改 UI

1. 打开 GitHub Desktop。
2. 切到自己的分支，例如 `feature/ui-menu`。
3. 点击 `Fetch origin`。
4. 如果有 `Pull origin`，点击它。
5. 打开 UE5。
6. 修改 UI。
7. 保存 UE5。
8. 回到 GitHub Desktop。
9. 检查 changed files。
10. 写 commit message。
11. Commit。
12. Push。
13. 创建 Pull Request。

### 18.2 我今天只交一张图

1. 把图片导入 UE5。
2. 放到正确目录。
3. 保存资源。
4. GitHub Desktop 检查 changed files。
5. 确认只包含相关资源。
6. Commit。
7. Push。
8. 如果需要合并到主分支，开 PR。

### 18.3 我今天只改音效

1. 把音频文件导入 UE5。
2. 放到 `Content/Audio/` 下。
3. 在 UI 或蓝图里替换引用。
4. Play 测试。
5. Commit。
6. Push。
7. 开 PR。

---

## 19. 如果出错了怎么办

### GitHub Desktop 显示很多奇怪文件

先不要提交。  
检查是不是把下面这些文件夹也提交了：

```text
Binaries
Intermediate
Saved
DerivedDataCache
```

如果看到了，先问负责 Git 的同学。

### Pull 的时候出现冲突

不要随便点 accept。  
尤其是 `.uasset` 和 `.umap` 冲突时，很容易覆盖别人工作。

正确做法：

1. 截图错误信息。
2. 发到群里。
3. 说明你改了哪些文件。
4. 等负责 Git 的同学一起处理。

### UE5 打不开资源

先尝试：

```powershell
git lfs pull
```

然后重新打开 UE5。

---

## 20. 团队约定

请大家遵守下面这些规则：

1. 不直接改主分支。
2. 每个人在自己的分支工作。
3. 改 `.umap` 前先和大家说。
4. 改大型 Widget 蓝图前先和大家说。
5. 提交前检查 changed files。
6. commit message 要写清楚。
7. 不提交 `Saved`、`Intermediate`、`Binaries`、`DerivedDataCache`。
8. 大文件必须走 Git LFS。
9. 不确定时先问，不要硬合并。
10. PR 描述要写测试情况。

---


## 21. 最短版流程


```text
Fetch / Pull 最新版本
切到自己的分支
修改项目
保存 UE5
检查 changed files
Commit
Push
Create Pull Request
```

---
