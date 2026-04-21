from pathlib import Path
import shutil
import subprocess
import sys

from jinja2 import Environment, FileSystemLoader
from PySide6.QtCore import Qt
from PySide6.QtWidgets import (
    QApplication,
    QCheckBox,
    QComboBox,
    QFormLayout,
    QGroupBox,
    QHBoxLayout,
    QLabel,
    QMainWindow,
    QMessageBox,
    QPushButton,
    QSpinBox,
    QTabWidget,
    QVBoxLayout,
    QWidget,
)


MAX_OSC_INST = 2
MAX_OSC_CHANNEL = 4
MAX_RULER_X_NUM = 5
MAX_RULER_Y_NUM = 5

THEME_OPTIONS = [
    ("OSC_THEME_DEFAULT", "Default"),
    ("OSC_THEME_LIGHT", "Light"),
]


def _spin(min_value: int, max_value: int, value: int) -> QSpinBox:
    box = QSpinBox()
    box.setRange(min_value, max_value)
    box.setValue(value)
    return box


class ProfileEditor(QWidget):
    def __init__(self, osc_idx: int, parent=None):
        super().__init__(parent)
        self.osc_idx = osc_idx
        self._build_ui()
        self._load_defaults()

    def _build_ui(self):
        root = QVBoxLayout(self)

        base_group = QGroupBox("基础参数")
        base_form = QFormLayout(base_group)
        self.x_origin = _spin(0, 65535, 0)
        self.y_origin = _spin(0, 65535, 0)
        self.x_width = _spin(1, 65535, 200)
        self.y_width = _spin(1, 65535, 120)
        self.display_min = _spin(0, 65535, 0)
        self.display_max = _spin(0, 65535, 4095)
        self.channel_num = _spin(1, MAX_OSC_CHANNEL, MAX_OSC_CHANNEL)
        self.theme = QComboBox()
        for value, text in THEME_OPTIONS:
            self.theme.addItem(text, value)
        self.is_auto_clear = QCheckBox("启用")
        self.is_auto_clear.setChecked(True)

        base_form.addRow("x_origin", self.x_origin)
        base_form.addRow("y_origin", self.y_origin)
        base_form.addRow("x_width", self.x_width)
        base_form.addRow("y_width", self.y_width)
        base_form.addRow("display_num_min", self.display_min)
        base_form.addRow("display_num_max", self.display_max)
        base_form.addRow("channel_num", self.channel_num)
        base_form.addRow("theme_type", self.theme)
        base_form.addRow("is_auto_clear", self.is_auto_clear)
        root.addWidget(base_group)

        ch_group = QGroupBox("通道使能")
        ch_layout = QHBoxLayout(ch_group)
        self.channel_checks = []
        for i in range(MAX_OSC_CHANNEL):
            c = QCheckBox(f"CH{i}")
            c.setChecked(True)
            self.channel_checks.append(c)
            ch_layout.addWidget(c)
        ch_layout.addStretch()
        root.addWidget(ch_group)

        ruler_y_group = QGroupBox("Y 标尺")
        ruler_y_layout = QFormLayout(ruler_y_group)
        self.is_display_ruler_y = QCheckBox("显示")
        self.is_display_ruler_y.setChecked(True)
        self.ruler_y_values = [_spin(0, 65535, 0) for _ in range(MAX_RULER_Y_NUM)]
        self.ruler_count_y = _spin(0, MAX_RULER_Y_NUM, 4)
        self.ruler_digits_y = _spin(0, 16, 4)
        y_values_layout = QHBoxLayout()
        for item in self.ruler_y_values:
            y_values_layout.addWidget(item)
        ruler_y_layout.addRow("is_display_ruler_y", self.is_display_ruler_y)
        ruler_y_layout.addRow("ruler_y[]", y_values_layout)
        ruler_y_layout.addRow("ruler_count_y", self.ruler_count_y)
        ruler_y_layout.addRow("ruler_num_digits_y", self.ruler_digits_y)
        root.addWidget(ruler_y_group)

        ruler_x_group = QGroupBox("X 标尺")
        ruler_x_layout = QFormLayout(ruler_x_group)
        self.is_display_ruler_x = QCheckBox("显示")
        self.is_display_ruler_x.setChecked(True)
        self.ruler_x_values = [_spin(0, 65535, 0) for _ in range(MAX_RULER_X_NUM)]
        self.ruler_count_x = _spin(0, MAX_RULER_X_NUM, 3)
        self.ruler_zero_x = _spin(0, 65535, 0)
        self.ruler_full_x = _spin(0, 65535, 100)
        self.ruler_digits_x = _spin(0, 16, 8)
        x_values_layout = QHBoxLayout()
        for item in self.ruler_x_values:
            x_values_layout.addWidget(item)
        ruler_x_layout.addRow("is_display_ruler_x", self.is_display_ruler_x)
        ruler_x_layout.addRow("ruler_x[]", x_values_layout)
        ruler_x_layout.addRow("ruler_count_x", self.ruler_count_x)
        ruler_x_layout.addRow("ruler_zero_value_x", self.ruler_zero_x)
        ruler_x_layout.addRow("ruler_full_value_x", self.ruler_full_x)
        ruler_x_layout.addRow("ruler_num_digits_x", self.ruler_digits_x)
        root.addWidget(ruler_x_group)
        root.addStretch()

    def _load_defaults(self):
        self.ruler_y_values[0].setValue(1000)
        self.ruler_y_values[1].setValue(2000)
        self.ruler_y_values[2].setValue(3000)
        self.ruler_y_values[3].setValue(4000)

        self.ruler_x_values[0].setValue(30)
        self.ruler_x_values[1].setValue(50)
        self.ruler_x_values[2].setValue(90)

        if self.osc_idx == 0:
            self.x_origin.setValue(10)
            self.y_origin.setValue(0)
            self.theme.setCurrentIndex(0)
        else:
            self.x_origin.setValue(0)
            self.y_origin.setValue(120)
            self.theme.setCurrentIndex(1)

    def to_dict(self) -> dict:
        enabled_bits = [i for i, c in enumerate(self.channel_checks) if c.isChecked()]
        mask_terms = [f"CH{i}" for i in enabled_bits]
        if not mask_terms:
            channel_mask_expr = "NO_CHANNELS_MASK"
        else:
            channel_mask_expr = " | ".join(mask_terms)

        return {
            "x_origin": self.x_origin.value(),
            "y_origin": self.y_origin.value(),
            "x_width": self.x_width.value(),
            "y_width": self.y_width.value(),
            "display_num_min": self.display_min.value(),
            "display_num_max": self.display_max.value(),
            "channel_num": self.channel_num.value(),
            "channel_mask_expr": channel_mask_expr,
            "is_display_ruler_y": "true" if self.is_display_ruler_y.isChecked() else "false",
            "ruler_y": [item.value() for item in self.ruler_y_values],
            "ruler_count_y": self.ruler_count_y.value(),
            "ruler_num_digits_y": self.ruler_digits_y.value(),
            "is_display_ruler_x": "true" if self.is_display_ruler_x.isChecked() else "false",
            "ruler_x": [item.value() for item in self.ruler_x_values],
            "ruler_count_x": self.ruler_count_x.value(),
            "ruler_zero_value_x": self.ruler_zero_x.value(),
            "ruler_full_value_x": self.ruler_full_x.value(),
            "ruler_num_digits_x": self.ruler_digits_x.value(),
            "theme_type": self.theme.currentData(),
            "is_auto_clear": "true" if self.is_auto_clear.isChecked() else "false",
        }


class MainWindow(QMainWindow):
    def __init__(self):
        super().__init__()
        self.setWindowTitle("OSC Profile Generator (V1)")
        self.resize(980, 760)
        self.repo_root = Path(__file__).resolve().parents[2]
        self.template_dir = Path(__file__).resolve().parent / "templates"
        self.output_file = self.repo_root / "core" / "OSC_Profile.c"
        self._build_ui()

    def _build_ui(self):
        root = QWidget()
        self.setCentralWidget(root)
        root_layout = QVBoxLayout(root)

        title = QLabel("生成目标: core/OSC_Profile.c")
        title.setAlignment(Qt.AlignmentFlag.AlignLeft)
        root_layout.addWidget(title)

        count_layout = QHBoxLayout()
        count_layout.addWidget(QLabel("osc_count"))
        self.osc_count = _spin(1, MAX_OSC_INST, MAX_OSC_INST)
        count_layout.addWidget(self.osc_count)
        count_layout.addStretch()
        root_layout.addLayout(count_layout)

        self.tabs = QTabWidget()
        self.editors = []
        for i in range(MAX_OSC_INST):
            editor = ProfileEditor(i)
            self.editors.append(editor)
            self.tabs.addTab(editor, f"OSC{i}")
        root_layout.addWidget(self.tabs)

        action_layout = QHBoxLayout()
        action_layout.addStretch()
        self.btn_generate = QPushButton("生成 OSC_Profile.c")
        self.btn_generate.clicked.connect(self.generate)
        action_layout.addWidget(self.btn_generate)
        self.btn_run = QPushButton("Run Simulator")
        self.btn_run.clicked.connect(self.run_simulator)
        action_layout.addWidget(self.btn_run)
        root_layout.addLayout(action_layout)

    def _validate(self, profiles: list[dict]) -> tuple[bool, str]:
        for idx, p in enumerate(profiles):
            if p["display_num_min"] >= p["display_num_max"]:
                return False, f"OSC{idx}: display_num_min 必须小于 display_num_max"
            if p["ruler_count_x"] > MAX_RULER_X_NUM:
                return False, f"OSC{idx}: ruler_count_x 超过上限"
            if p["ruler_count_y"] > MAX_RULER_Y_NUM:
                return False, f"OSC{idx}: ruler_count_y 超过上限"
        return True, ""

    def generate(self, show_success: bool = True) -> bool:
        osc_count = self.osc_count.value()
        profiles = [editor.to_dict() for editor in self.editors[:osc_count]]
        ok, message = self._validate(profiles)
        if not ok:
            QMessageBox.warning(self, "参数错误", message)
            return False

        env = Environment(loader=FileSystemLoader(str(self.template_dir)), trim_blocks=True, lstrip_blocks=True)
        template = env.get_template("OSC_Profile.c.j2")
        content = template.render(osc_count=osc_count, profiles=profiles)

        if self.output_file.exists():
            backup = self.output_file.with_suffix(".c.bak")
            shutil.copyfile(self.output_file, backup)

        self.output_file.write_text(content, encoding="utf-8")
        if show_success:
            QMessageBox.information(self, "完成", f"已生成: {self.output_file}")
        return True

    def run_simulator(self):
        if not self.generate(show_success=False):
            return

        try:
            build_cmd = ["cmake", "-B", "build"]
            subprocess.run(build_cmd, cwd=self.repo_root, check=True)

            compile_cmd = ["cmake", "--build", "build"]
            subprocess.run(compile_cmd, cwd=self.repo_root, check=True)

            exe = self.repo_root / "build" / "OSC_Simulator"
            if not exe.exists():
                QMessageBox.critical(self, "运行失败", f"未找到可执行文件: {exe}")
                return

            subprocess.Popen([str(exe)], cwd=self.repo_root)
            QMessageBox.information(self, "已启动", "OSC_Simulator 已启动。")
        except subprocess.CalledProcessError as exc:
            QMessageBox.critical(self, "运行失败", f"命令执行失败: {exc}")
        except Exception as exc:
            QMessageBox.critical(self, "运行失败", str(exc))


def main():
    app = QApplication(sys.argv)
    window = MainWindow()
    window.show()
    sys.exit(app.exec())


if __name__ == "__main__":
    main()
