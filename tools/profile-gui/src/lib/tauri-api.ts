import { invoke } from "@tauri-apps/api/core";
import type { ProfileSet } from "./types";

export async function loadProfile(): Promise<ProfileSet> {
  return invoke("load_profile");
}

export async function saveProfile(data: ProfileSet): Promise<void> {
  return invoke("save_profile", { data });
}

export async function buildSimulator(): Promise<string> {
  return invoke("build_simulator");
}

export async function runSimulator(): Promise<void> {
  return invoke("run_simulator");
}
