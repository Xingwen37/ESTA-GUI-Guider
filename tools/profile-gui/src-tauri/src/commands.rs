use std::path::PathBuf;
use std::process::Command;
use serde_json::json;
use tauri::State;
use crate::models::{EstaProfile, ProfileSet};

const PROFILE_JSON: &str = "core/ESTA_Profile.json";
const PROFILE_C: &str = "core/ESTA_Profile.c";
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
        serde_json::from_str(&content).map_err(|e| format!("JSON 解析失败: {}", e))
    } else {
        Ok(default_profile())
    }
}

#[tauri::command]
pub fn save_profile(state: State<AppState>, data: ProfileSet) -> Result<(), String> {
    let json_path = state.repo_root.join(PROFILE_JSON);
    let c_path = state.repo_root.join(PROFILE_C);

    // Build template context using serde_json::Value
    let profiles_for_template: Vec<serde_json::Value> = data
        .profiles
        .iter()
        .map(|p| {
            json!({
                "x_origin": p.x_origin,
                "y_origin": p.y_origin,
                "x_width": p.x_width,
                "y_width": p.y_width,
                "display_num_min": p.display_num_min,
                "display_num_max": p.display_num_max,
                "channel_num": p.channel_num,
                "channel_mask_expr": p.channel_mask_expr(),
                "is_display_ruler_y": p.is_display_ruler_y,
                "ruler_y": p.ruler_y,
                "ruler_count_y": p.ruler_count_y,
                "ruler_num_digits_y": p.ruler_num_digits_y,
                "is_display_ruler_x": p.is_display_ruler_x,
                "ruler_x": p.ruler_x,
                "ruler_count_x": p.ruler_count_x,
                "ruler_zero_value_x": p.ruler_zero_value_x,
                "ruler_full_value_x": p.ruler_full_value_x,
                "ruler_num_digits_x": p.ruler_num_digits_x,
                "theme_type": p.theme_type,
                "is_auto_clear": p.is_auto_clear,
            })
        })
        .collect();

    let mut ctx = tera::Context::new();
    ctx.insert("inst_count", &data.inst_count);
    ctx.insert("bar_inst_count", &data.bar_inst_count);
    ctx.insert("profiles", &profiles_for_template);

    // Render C code from template
    let c_code = state
        .tera
        .render(TEMPLATE, &ctx)
        .map_err(|e| format!("模板渲染失败: {}", e))?;

    // Backup existing C file
    if c_path.exists() {
        let backup = c_path.with_extension("c.bak");
        std::fs::copy(&c_path, &backup).map_err(|e| e.to_string())?;
    }

    // Write C file
    std::fs::write(&c_path, c_code).map_err(|e| e.to_string())?;

    // Write JSON
    let json = serde_json::to_string_pretty(&data).map_err(|e| e.to_string())?;
    std::fs::write(&json_path, json).map_err(|e| e.to_string())?;

    Ok(())
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
        inst_count: 2,
        bar_inst_count: 1,
        profiles: vec![
            EstaProfile {
                x_origin: 10, y_origin: 0, x_width: 200, y_width: 120,
                display_num_min: 0, display_num_max: 4095,
                channel_num: 4, channel_mask: 0b00001001,
                is_display_ruler_y: true,
                ruler_y: [1000, 2000, 3000, 4000, 0],
                ruler_count_y: 4, ruler_num_digits_y: 4,
                is_display_ruler_x: true,
                ruler_x: [30, 50, 90, 0, 0],
                ruler_count_x: 3, ruler_zero_value_x: 0, ruler_full_value_x: 100,
                ruler_num_digits_x: 8,
                theme_type: "WAVE_THEME_DEFAULT".into(),
                is_auto_clear: true,
                bar_x_origin: 10,
                bar_y_origin: 125,
                bar_x_width: 300,
                bar_y_width: 110,
                bar_display_num_min: 0,
                bar_display_num_max: 100,
                bar_count: 6,
                bar_width: 0,
                bar_spacing: 0,
                bar_is_display_value: true,
                bar_is_display_axis: true,
                bar_theme_type: "BARCHART_THEME_DEFAULT".into(),
            },
            EstaProfile {
                x_origin: 0, y_origin: 120, x_width: 200, y_width: 120,
                display_num_min: 0, display_num_max: 4095,
                channel_num: 4, channel_mask: 0b00001111,
                is_display_ruler_y: true,
                ruler_y: [1000, 2000, 3000, 4000, 0],
                ruler_count_y: 4, ruler_num_digits_y: 4,
                is_display_ruler_x: true,
                ruler_x: [30, 50, 90, 0, 0],
                ruler_count_x: 3, ruler_zero_value_x: 0, ruler_full_value_x: 100,
                ruler_num_digits_x: 8,
                theme_type: "WAVE_THEME_LIGHT".into(),
                is_auto_clear: true,
                bar_x_origin: 10,
                bar_y_origin: 125,
                bar_x_width: 300,
                bar_y_width: 110,
                bar_display_num_min: 0,
                bar_display_num_max: 100,
                bar_count: 6,
                bar_width: 0,
                bar_spacing: 0,
                bar_is_display_value: true,
                bar_is_display_axis: true,
                bar_theme_type: "BARCHART_THEME_DEFAULT".into(),
            },
        ],
    }
}
