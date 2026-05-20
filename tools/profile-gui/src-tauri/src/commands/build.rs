use std::path::Path;
use std::process::Command;

use base64::Engine as _;
use tauri::State;

use crate::models::ProfileSet;

use super::profile_io::save_profile_internal;
use super::AppState;

pub fn build_simulator_internal(repo_root: &Path) -> Result<(), String> {
    let output = Command::new("cmake")
        .args(["-G", "MinGW Makefiles", "-B", "build", "-S", "."])
        .current_dir(repo_root)
        .output()
        .map_err(|e| format!("cmake 配置失败: {}", e))?;
    if !output.status.success() {
        return Err(String::from_utf8_lossy(&output.stderr).to_string());
    }
    let output = Command::new("cmake")
        .args(["--build", "build"])
        .current_dir(repo_root)
        .output()
        .map_err(|e| format!("cmake 构建失败: {}", e))?;
    if !output.status.success() {
        return Err(String::from_utf8_lossy(&output.stderr).to_string());
    }
    Ok(())
}

#[tauri::command]
pub fn build_simulator(state: State<AppState>) -> Result<String, String> {
    build_simulator_internal(&state.repo_root)?;
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

#[tauri::command]
pub fn preview_simulator(state: State<AppState>, data: ProfileSet) -> Result<Vec<String>, String> {
    let page_count = data.page_count.max(1);

    save_profile_internal(&state.repo_root, &state.tera, &state.registry, &data)?;
    build_simulator_internal(&state.repo_root)?;

    let exe = state.repo_root.join("build").join("ESTA_Simulator.exe");
    let preview_base = state.repo_root.join("build").join("preview");

    let output = Command::new(&exe)
        .args([
            "--screenshot",
            preview_base.to_str().unwrap_or("build/preview"),
        ])
        .current_dir(&state.repo_root)
        .output()
        .map_err(|e| format!("预览失败: {}", e))?;
    if !output.status.success() {
        return Err(format!(
            "仿真器截图失败: {}",
            String::from_utf8_lossy(&output.stderr)
        ));
    }

    let mut pages = Vec::new();
    for p in 0..page_count {
        let path = format!("{}_{}.bmp", preview_base.display(), p);
        let bmp_data =
            std::fs::read(&path).map_err(|e| format!("读取第{}页截图失败: {}", p, e))?;
        let b64 = base64::engine::general_purpose::STANDARD.encode(&bmp_data);
        pages.push(format!("data:image/bmp;base64,{}", b64));
    }
    Ok(pages)
}
