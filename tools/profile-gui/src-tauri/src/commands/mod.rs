use std::path::PathBuf;

use crate::registry::Registry;

pub mod build;
pub mod profile_io;

pub const PROFILE_JSON: &str = "core/profile/ESTA_Profile.json";
pub const PROFILE_C: &str = "core/profile/ESTA_Profile.c";
pub const TEMPLATE: &str = "ESTA_Profile.c.j2";

pub struct AppState {
    pub repo_root: PathBuf,
    pub tera: tera::Tera,
    pub registry: Registry,
}

pub fn repo_root() -> PathBuf {
    std::env::current_dir()
        .unwrap_or_default()
        .ancestors()
        .find(|p| p.join("CMakeLists.txt").exists())
        .map(PathBuf::from)
        .unwrap_or_else(|| PathBuf::from("."))
}
