mod commands;
mod defaults;
mod models;
mod plugin;
mod plugins;
mod registry;
mod util;

use commands::AppState;
use registry::Registry;

#[cfg_attr(mobile, tauri::mobile_entry_point)]
pub fn run() {
    let repo_root = commands::repo_root();
    let template_path = repo_root
        .join("tools")
        .join("profile-gui")
        .join("src-tauri")
        .join("templates")
        .join("*.j2");
    let template_glob = template_path.to_string_lossy().to_string();

    let tera = match tera::Tera::new(&template_glob) {
        Ok(t) => t,
        Err(e) => {
            eprintln!("模板加载失败: {}", e);
            std::process::exit(1);
        }
    };

    let mut registry = Registry::new();
    registry.register(Box::new(plugins::wave::WavePlugin));
    registry.register(Box::new(plugins::bar::BarChartPlugin));
    registry.register(Box::new(plugins::table::TablePlugin));
    registry.register(Box::new(plugins::menu::MenuPlugin));
    registry.register(Box::new(plugins::flag::FlagPlugin));

    tauri::Builder::default()
        .manage(AppState {
            repo_root,
            tera,
            registry,
        })
        .invoke_handler(tauri::generate_handler![
            commands::profile_io::load_profile,
            commands::profile_io::save_profile,
            commands::build::build_simulator,
            commands::build::run_simulator,
            commands::build::preview_simulator,
        ])
        .run(tauri::generate_context!())
        .expect("error while running tauri application");
}
