import type { ActionSequence, ActionStep } from "../lib/types";
import {
  TARGET_TYPE_OPTIONS,
  ACTION_TYPE_OPTIONS,
  MAX_SEQUENCES,
  MAX_SEQUENCE_STEPS,
} from "../lib/types";

interface Props {
  sequences: ActionSequence[];
  waveInstCount: number;
  barInstCount: number;
  tableInstCount: number;
  menuInstCount: number;
  onChange: (sequences: ActionSequence[]) => void;
}

const EMPTY_STEP: ActionStep = {
  action: 1,
  target_type: 4,
  target_inst: 0,
  param: 0,
};

const EMPTY_SEQUENCE: ActionSequence = {
  step_count: 0,
  steps: [],
};

const STEP_ACTION_OPTIONS = ACTION_TYPE_OPTIONS.filter(([v]) => v !== 11);

function getInstCount(targetType: number, props: Props): number {
  switch (targetType) {
    case 0: return props.waveInstCount;
    case 1: return props.barInstCount;
    case 2: return props.tableInstCount;
    case 3: return props.menuInstCount;
    case 6: return 8;
    default: return 0;
  }
}

function isSingleton(targetType: number): boolean {
  return targetType === 4 || targetType === 5;
}

export default function SequenceEditor(props: Props) {
  const { sequences, onChange } = props;

  const addSequence = () => {
    if (sequences.length >= MAX_SEQUENCES) return;
    onChange([...sequences, { ...EMPTY_SEQUENCE }]);
  };

  const removeSequence = (si: number) => {
    onChange(sequences.filter((_, i) => i !== si));
  };

  const addStep = (si: number) => {
    const seq = sequences[si];
    if (seq.steps.length >= MAX_SEQUENCE_STEPS) return;
    const newSteps = [...seq.steps, { ...EMPTY_STEP }];
    updateSequence(si, { ...seq, steps: newSteps, step_count: newSteps.length });
  };

  const removeStep = (si: number, stepIdx: number) => {
    const seq = sequences[si];
    const newSteps = seq.steps.filter((_, i) => i !== stepIdx);
    updateSequence(si, { ...seq, steps: newSteps, step_count: newSteps.length });
  };

  const updateStep = (si: number, stepIdx: number, field: keyof ActionStep, value: number) => {
    const seq = sequences[si];
    const newSteps = [...seq.steps];
    const updated = { ...newSteps[stepIdx], [field]: value };
    if (field === "target_type" && isSingleton(value)) {
      updated.target_inst = 0;
    }
    newSteps[stepIdx] = updated;
    updateSequence(si, { ...seq, steps: newSteps });
  };

  const updateSequence = (si: number, updated: ActionSequence) => {
    const next = [...sequences];
    next[si] = updated;
    onChange(next);
  };

  return (
    <div style={{ padding: 12 }}>
      <fieldset className="group-box">
        <legend>Action 序列 ({sequences.length}/{MAX_SEQUENCES})</legend>
        {sequences.map((seq, si) => (
          <div key={si} style={{ marginBottom: 12, border: "1px solid #444", borderRadius: 4, padding: 8 }}>
            <div style={{ display: "flex", alignItems: "center", marginBottom: 6, gap: 8 }}>
              <span style={{ fontWeight: "bold", fontSize: 13 }}>Seq #{si}</span>
              <span style={{ color: "#888", fontSize: 12 }}>{seq.step_count} steps</span>
              <button onClick={() => removeSequence(si)}
                style={{ marginLeft: "auto", background: "none", border: "none", color: "#e55", cursor: "pointer", fontSize: 14 }}>
                &times; 删除序列
              </button>
            </div>
            <table style={{ width: "100%", borderCollapse: "collapse", fontSize: 12 }}>
              <thead>
                <tr style={{ borderBottom: "1px solid #444" }}>
                  <th style={{ textAlign: "left", padding: "3px 5px" }}>#</th>
                  <th style={{ textAlign: "left", padding: "3px 5px" }}>Action</th>
                  <th style={{ textAlign: "left", padding: "3px 5px" }}>Target</th>
                  <th style={{ textAlign: "left", padding: "3px 5px" }}>Inst</th>
                  <th style={{ textAlign: "left", padding: "3px 5px" }}>Param</th>
                  <th style={{ width: 30 }}></th>
                </tr>
              </thead>
              <tbody>
                {seq.steps.map((step, stepIdx) => {
                  const singleton = isSingleton(step.target_type);
                  const instCount = getInstCount(step.target_type, props);
                  return (
                    <tr key={stepIdx} style={{ borderBottom: "1px solid #333" }}>
                      <td style={{ padding: "3px 5px", color: "#888" }}>{stepIdx}</td>
                      <td style={{ padding: "3px 5px" }}>
                        <select value={step.action}
                          onChange={(e) => updateStep(si, stepIdx, "action", Number(e.target.value))}>
                          {STEP_ACTION_OPTIONS.map(([val, label]) => (
                            <option key={val} value={val}>{label}</option>
                          ))}
                        </select>
                      </td>
                      <td style={{ padding: "3px 5px" }}>
                        <select value={step.target_type}
                          onChange={(e) => updateStep(si, stepIdx, "target_type", Number(e.target.value))}>
                          {TARGET_TYPE_OPTIONS.map(([val, label]) => (
                            <option key={val} value={val}>{label}</option>
                          ))}
                        </select>
                      </td>
                      <td style={{ padding: "3px 5px" }}>
                        {singleton ? (
                          <span style={{ color: "#888" }}>—</span>
                        ) : (
                          <select value={step.target_inst}
                            onChange={(e) => updateStep(si, stepIdx, "target_inst", Number(e.target.value))}>
                            {Array.from({ length: Math.max(instCount, 1) }, (_, k) => (
                              <option key={k} value={k}>#{k}</option>
                            ))}
                          </select>
                        )}
                      </td>
                      <td style={{ padding: "3px 5px" }}>
                        <input type="number" value={step.param} min={0} max={255}
                          style={{ width: 46 }}
                          onChange={(e) => updateStep(si, stepIdx, "param", Number(e.target.value) || 0)} />
                      </td>
                      <td style={{ padding: "3px 5px", textAlign: "center" }}>
                        <button onClick={() => removeStep(si, stepIdx)} title="删除步骤"
                          style={{ background: "none", border: "none", color: "#e55", cursor: "pointer", fontSize: 14 }}>
                          &times;
                        </button>
                      </td>
                    </tr>
                  );
                })}
              </tbody>
            </table>
            {seq.steps.length < MAX_SEQUENCE_STEPS && (
              <button onClick={() => addStep(si)} style={{ marginTop: 6, fontSize: 12 }}>+ 添加步骤</button>
            )}
            {seq.steps.length === 0 && (
              <div style={{ color: "#999", padding: "6px 0", fontSize: 12 }}>暂无步骤</div>
            )}
          </div>
        ))}
        {sequences.length < MAX_SEQUENCES && (
          <button onClick={addSequence} style={{ marginTop: 4 }}>+ 添加序列</button>
        )}
        {sequences.length === 0 && (
          <div style={{ color: "#999", padding: 12 }}>暂无序列，SEQUENCE action 需要引用此表</div>
        )}
      </fieldset>
    </div>
  );
}
