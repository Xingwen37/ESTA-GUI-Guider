mod commands;
mod models;

use commands::AppState;

#[cfg_attr(mobile, tauri::mobile_entry_point)]
pub fn run() {
    let repo_root = commands::repo_root();
    let template_path = repo_root.join("tools").join("profile-gui").join("src-tauri").join("templates").join("*.j2");
    let template_glob = template_path.to_string_lossy().to_string();

    let tera = match tera::Tera::new(&template_glob) {
        Ok(t) => t,
        Err(e) => {
            eprintln!("模板加载失败: {}", e);
            std::process::exit(1);
        }
    };

    tauri::Builder::default()
        .manage(AppState { repo_root, tera })
        .invoke_handler(tauri::generate_handler![
            commands::load_profile,
            commands::save_profile,
            commands::build_simulator,
            commands::run_simulator,
        ])
        .run(tauri::generate_context!())
        .expect("error while running tauri application");
}
