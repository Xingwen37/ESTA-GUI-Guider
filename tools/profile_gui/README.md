# OSC Profile GUI (V1)

## 1) 安装依赖
```bash
cd tools/profile_gui
python -m venv .venv
source .venv/bin/activate
pip install -r requirements.txt
```

## 2) 启动
```bash
cd tools/profile_gui
source .venv/bin/activate
python app.py
```

## 3) 生成结果
- 输出文件：`core/OSC_Profile.c`
- 备份文件：`core/OSC_Profile.c.bak`（若原文件存在）
