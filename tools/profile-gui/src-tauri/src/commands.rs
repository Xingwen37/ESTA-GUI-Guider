use std::path::PathBuf;
use std::process::Command;

use serde_json::json;
use tauri::State;

use crate::models::{
    BarChartProfile, ProfileSet, TableProfile, TableRowProfile, WaveProfile,
    WaveRulerLabelProfile,
};

const PROFILE_JSON: &str = "core/profile/ESTA_Profile.json";
const PROFILE_C: &str = "core/profile/ESTA_Profile.c";
const TEMPLATE: &str = "ESTA_Profile.c.j2";

pub struct AppState {
    pub repo_root: PathBuf,
    pub tera: tera::Tera,
}

pub fn repo_root() -> PathBuf {
    std::env::current_dir()
        .unwrap_or_default()
        .ancestors()
        .find(|p| p.join("CMakeLists.txt").exists())
        .map(PathBuf::from)
        .unwrap_or_else(|| PathBuf::from("."))
}

#[tauri::command]
pub fn load_profile(state: State<AppState>) -> Result<ProfileSet, String> {
    let json_path = state.repo_root.join(PROFILE_JSON);
    if json_path.exists() {
        let mut content = std::fs::read_to_string(&json_path).map_err(|e| e.to_string())?;
        // Strip UTF-8 BOM if present (Windows editors may add it)
        if content.starts_with('\u{FEFF}') {
            content.remove(0);
        }
        let has_ruler_label_y = content.contains("\"ruler_label_y\"");
        let has_ruler_label_x = content.contains("\"ruler_label_x\"");
        let mut profile: ProfileSet =
            serde_json::from_str(&content).map_err(|e| format!("JSON 解析失败: {}", e))?;
        normalize_wave_ruler_labels(&mut profile, has_ruler_label_y, has_ruler_label_x);
        Ok(profile)
    } else {
        Ok(default_profile())
    }
}

#[tauri::command]
pub fn save_profile(state: State<AppState>, data: ProfileSet) -> Result<(), String> {
    let json_path = state.repo_root.join(PROFILE_JSON);
    let c_path = state.repo_root.join(PROFILE_C);

    let wave_profiles_for_template: Vec<serde_json::Value> = data
        .wave_profiles
        .iter()
        .map(|p| {
            json!({
                "x_origin": p.x_origin,
                "y_origin": p.y_origin,
                "x_width": p.x_width,
                "y_width": p.y_width,
                "display_num_min": p.display_num_min,
                "display_num_max": p.display_num_max,
                "x_scale": p.x_scale,
                "channel_num": p.channel_num,
                "channel_mask_expr": p.channel_mask_expr(),
                "is_display_ruler_y": p.is_display_ruler_y,
                "ruler_y": p.ruler_y,
                "ruler_label_y": wave_labels_for_template(&p.ruler_label_y),
                "ruler_unit_y": c_string_literal(&p.ruler_unit_y),
                "ruler_precision_y": p.ruler_precision_y,
                "ruler_count_y": p.ruler_count_y,
                "ruler_num_digits_y": p.ruler_num_digits_y,
                "ruler_font_size_y": p.ruler_font_size_y,
                "is_display_ruler_x": p.is_display_ruler_x,
                "ruler_x": p.ruler_x,
                "ruler_label_x": wave_labels_for_template(&p.ruler_label_x),
                "ruler_unit_x": c_string_literal(&p.ruler_unit_x),
                "ruler_precision_x": p.ruler_precision_x,
                "ruler_count_x": p.ruler_count_x,
                "ruler_zero_value_x": p.ruler_zero_value_x,
                "ruler_full_value_x": p.ruler_full_value_x,
                "ruler_num_digits_x": p.ruler_num_digits_x,
                "ruler_font_size_x": p.ruler_font_size_x,
                "theme_type": p.theme_type,
                "is_auto_clear": p.is_auto_clear,
                "is_use_batch_draw": p.is_use_batch_draw,
            })
        })
        .collect();

    let bar_profiles_for_template: Vec<serde_json::Value> = data
        .bar_profiles
        .iter()
        .map(|p| {
            json!({
                "x_origin": p.x_origin,
                "y_origin": p.y_origin,
                "x_width": p.x_width,
                "y_width": p.y_width,
                "display_num_min": p.display_num_min,
                "display_num_max": p.display_num_max,
                "bar_count": p.bar_count,
                "bar_width": p.bar_width,
                "bar_spacing": p.bar_spacing,
                "is_display_value": p.is_display_value,
                "is_display_axis": p.is_display_axis,
                "font_size": p.font_size,
                "theme_type": p.theme_type,
            })
        })
        .collect();

    let table_profiles_for_template: Vec<serde_json::Value> = data
        .table_profiles
        .iter()
        .map(|p| {
            let rows: Vec<serde_json::Value> = p
                .rows
                .iter()
                .map(|row| {
                    json!({
                        "label": c_string_literal(&row.label),
                        "value_kind": row.value_kind,
                        "number_type": row.number_type,
                        "unit": c_string_literal(&row.unit),
                        "precision": row.precision,
                        "default_u32": row.default_u32,
                        "default_float": row.default_float,
                        "default_text": c_string_literal(&row.default_text),
                    })
                })
                .collect();
            json!({
                "x_origin": p.x_origin,
                "y_origin": p.y_origin,
                "x_width": p.x_width,
                "y_width": p.y_width,
                "row_count": p.row_count,
                "row_height": p.row_height,
                "label_col_width": p.label_col_width,
                "value_col_width": p.value_col_width,
                "unit_col_width": p.unit_col_width,
                "is_auto_col_width": p.is_auto_col_width,
                "is_show_frame": p.is_show_frame,
                "is_show_row_line": p.is_show_row_line,
                "is_fill_background": p.is_fill_background,
                "font_size": p.font_size,
                "theme_type": p.theme_type,
                "rows": rows,
            })
        })
        .collect();

    let mut ctx = tera::Context::new();
    ctx.insert("wave_inst_count", &data.wave_inst_count);
    ctx.insert("bar_inst_count", &data.bar_inst_count);
    ctx.insert("table_inst_count", &data.table_inst_count);
    ctx.insert("button_count", &data.button_count);
    ctx.insert("wave_profiles", &wave_profiles_for_template);
    ctx.insert("bar_profiles", &bar_profiles_for_template);
    ctx.insert("table_profiles", &table_profiles_for_template);

    let c_code = state
        .tera
        .render(TEMPLATE, &ctx)
        .map_err(|e| format!("模板渲染失败: {}", e))?;

    if c_path.exists() {
        let backup = c_path.with_extension("c.bak");
        std::fs::copy(&c_path, &backup).map_err(|e| e.to_string())?;
    }

    std::fs::write(&c_path, c_code).map_err(|e| e.to_string())?;

    let json = serde_json::to_string_pretty(&data).map_err(|e| e.to_string())?;
    std::fs::write(&json_path, json).map_err(|e| e.to_string())?;

    Ok(())
}

fn c_string_literal(value: &str) -> String {
    let mut out = String::from("\"");
    for ch in value.chars().take(16) {
        match ch {
            '\\' => out.push_str("\\\\"),
            '"' => out.push_str("\\\""),
            '\n' => out.push_str("\\n"),
            '\r' => out.push_str("\\r"),
            '\t' => out.push_str("\\t"),
            c if c.is_ascii_graphic() || c == ' ' => out.push(c),
            _ => out.push('?'),
        }
    }
    out.push('"');
    out
}

fn wave_label_from_u16(value: u16) -> WaveRulerLabelProfile {
    WaveRulerLabelProfile {
        value_type: "WAVE_RULER_LABEL_INT".into(),
        int_value: i32::from(value),
        float_value: f32::from(value),
    }
}

fn wave_labels_from_positions(values: &[u16; 10]) -> [WaveRulerLabelProfile; 10] {
    std::array::from_fn(|i| wave_label_from_u16(values[i]))
}

fn wave_labels_for_template(labels: &[WaveRulerLabelProfile; 10]) -> Vec<serde_json::Value> {
    labels
        .iter()
        .map(|label| {
            json!({
                "value_type": label.value_type.clone(),
                "int_value": label.int_value,
                "float_value": label.float_value,
            })
        })
        .collect()
}

fn normalize_wave_ruler_labels(
    profile: &mut ProfileSet,
    has_ruler_label_y: bool,
    has_ruler_label_x: bool,
) {
    for wave in &mut profile.wave_profiles {
        if !has_ruler_label_y {
            wave.ruler_label_y = wave_labels_from_positions(&wave.ruler_y);
            wave.ruler_unit_y.clear();
            wave.ruler_precision_y = 0;
        }
        if !has_ruler_label_x {
            wave.ruler_label_x = wave_labels_from_positions(&wave.ruler_x);
            wave.ruler_unit_x.clear();
            wave.ruler_precision_x = 0;
        }
    }
}

#[tauri::command]
pub fn build_simulator(state: State<AppState>) -> Result<String, String> {
    let root = &state.repo_root;

    let output = Command::new("cmake")
        .args(["-G", "MinGW Makefiles", "-B", "build", "-S", "."])
        .current_dir(root)
        .output()
        .map_err(|e| format!("cmake 配置失败: {}", e))?;

    if !output.status.success() {
        return Err(String::from_utf8_lossy(&output.stderr).to_string());
    }

    let output = Command::new("cmake")
        .args(["--build", "build"])
        .current_dir(root)
        .output()
        .map_err(|e| format!("cmake 构建失败: {}", e))?;

    if !output.status.success() {
        return Err(String::from_utf8_lossy(&output.stderr).to_string());
    }

    Ok("构建成功".into())
}

#[tauri::command]
pub fn run_simulator(state: State<AppState>) -> Result<(), String> {
    let exe = state.repo_root.join("build").join("ESTA_Simulator.exe");
    if !exe.exists() {
        return Err(format!("未找到可执行文件: {}", exe.display()));
    }
    Command::new(exe)
        .current_dir(&state.repo_root)
        .spawn()
        .map_err(|e| format!("启动失败: {}", e))?;
    Ok(())
}

fn default_profile() -> ProfileSet {
    ProfileSet {
        wave_inst_count: 2,
        bar_inst_count: 1,
        table_inst_count: 1,
        button_count: 4,
        wave_profiles: vec![
            default_wave_profile(10, 0, "WAVE_THEME_LIGHT", true),
            default_wave_profile(0, 125, "WAVE_THEME_DEFAULT", true),
        ],
        bar_profiles: vec![default_bar_profile(
            200,
            125,
            130,
            110,
            120,
            20,
            "BARCHART_THEME_LIGHT",
        )],
        table_profiles: vec![default_table_profile()],
    }
}

fn default_wave_profile(
    x_origin: u16,
    y_origin: u16,
    theme_type: &str,
    is_use_batch_draw: bool,
) -> WaveProfile {
    let ruler_y = [1000, 2000, 3000, 4000, 0, 0, 0, 0, 0, 0];
    let ruler_x = [30, 50, 90, 0, 0, 0, 0, 0, 0, 0];
    WaveProfile {
        x_origin,
        y_origin,
        x_width: 200,
        y_width: 120,
        display_num_min: 0,
        display_num_max: 4095,
        x_scale: 1,
        channel_num: 4,
        channel_mask: 0b00001111,
        is_display_ruler_y: true,
        ruler_y,
        ruler_label_y: wave_labels_from_positions(&ruler_y),
        ruler_unit_y: "".into(),
        ruler_precision_y: 0,
        ruler_count_y: 4,
        ruler_num_digits_y: 4,
        ruler_font_size_y: "ESTA_FONT_1608".into(),
        is_display_ruler_x: true,
        ruler_x,
        ruler_label_x: wave_labels_from_positions(&ruler_x),
        ruler_unit_x: "".into(),
        ruler_precision_x: 0,
        ruler_count_x: 3,
        ruler_zero_value_x: 0,
        ruler_full_value_x: 100,
        ruler_num_digits_x: 8,
        ruler_font_size_x: "ESTA_FONT_1608".into(),
        theme_type: theme_type.into(),
        is_auto_clear: true,
        is_use_batch_draw,
    }
}

fn default_table_profile() -> TableProfile {
    TableProfile {
        x_origin: 210,
        y_origin: 0,
        x_width: 110,
        y_width: 72,
        row_count: 3,
        row_height: 20,
        label_col_width: 32,
        value_col_width: 48,
        unit_col_width: 24,
        is_auto_col_width: true,
        is_show_frame: true,
        is_show_row_line: false,
        is_fill_background: true,
        font_size: "ESTA_FONT_1608".into(),
        theme_type: "TABLE_THEME_LIGHT".into(),
        rows: vec![
            TableRowProfile {
                label: "Vpp".into(),
                value_kind: "TABLE_VALUE_NUMBER".into(),
                number_type: "TABLE_NUMBER_UINT32".into(),
                unit: "mV".into(),
                precision: 0,
                default_u32: 1000,
                default_float: 0.0,
                default_text: "".into(),
            },
            TableRowProfile {
                label: "Fre".into(),
                value_kind: "TABLE_VALUE_NUMBER".into(),
                number_type: "TABLE_NUMBER_UINT32".into(),
                unit: "Hz".into(),
                precision: 0,
                default_u32: 1230,
                default_float: 0.0,
                default_text: "".into(),
            },
            TableRowProfile {
                label: "Mode".into(),
                value_kind: "TABLE_VALUE_TEXT".into(),
                number_type: "TABLE_NUMBER_UINT32".into(),
                unit: "".into(),
                precision: 0,
                default_u32: 0,
                default_float: 0.0,
                default_text: "AUTO".into(),
            },
        ],
    }
}

fn default_bar_profile(
    x_origin: u16,
    y_origin: u16,
    x_width: u16,
    y_width: u16,
    display_num_max: u16,
    bar_width: u16,
    theme_type: &str,
) -> BarChartProfile {
    BarChartProfile {
        x_origin,
        y_origin,
        x_width,
        y_width,
        display_num_min: 0,
        display_num_max,
        bar_count: 6,
        bar_width,
        bar_spacing: 0,
        is_display_value: true,
        is_display_axis: true,
        font_size: "ESTA_FONT_1608".into(),
        theme_type: theme_type.into(),
    }
}
