"use client";
import React, { useState, useEffect } from 'react';
import { X, Settings2, Ruler, ArrowUpDown, Maximize2, Clock, ShieldAlert, Sparkles, Check, AlertCircle } from 'lucide-react';

interface Props {
  show: boolean;
  onClose: () => void;
  onSubmit: (e: React.FormEvent) => void;
  settings: any;
  setSettings: (val: any) => void;
}

// ข้อจำกัดทางฮาร์ดแวร์และความปลอดภัย (Constraints)
export const SETTING_LIMITS = {
  stay_timeout_minutes: { min: 1.0, max: 60.0, step: 1.0, unit: "นาที", label: "แจ้งเตือนอยู่นานเกินกำหนด" },
  mount_height: { min: 2.0, max: 3.0, step: 0.1, unit: "เมตร", label: "ความสูงติดตั้งจากพื้น" },
  room_width: { min: 0.8, max: 4.0, step: 0.1, unit: "เมตร", label: "ความกว้างห้องน้ำ" },
  room_length: { min: 0.8, max: 4.0, step: 0.1, unit: "เมตร", label: "ความยาวห้องน้ำ" },
  fall_threshold: { min: 0.2, max: 0.8, step: 0.05, unit: "เมตร", label: "เกณฑ์ความสูงการล้ม" },
};

export default function DeviceSettingsModal({ show, onClose, onSubmit, settings, setSettings }: Props) {
  const [errors, setErrors] = useState<{ [key: string]: string }>({});

  // ตรวจสอบความถูกต้องของข้อมูลทุกครั้งที่ค่าเปลี่ยน
  useEffect(() => {
    if (!show) return;

    const newErrors: { [key: string]: string } = {};

    // 1. ตรวจสอบ Stay Timeout
    const timeout = Number(settings.stay_timeout_minutes);
    if (isNaN(timeout) || settings.stay_timeout_minutes === "" || timeout <= 0) {
      newErrors.stay_timeout_minutes = "ต้องเป็นตัวเลขบวก และห้ามติดลบหรือเท่ากับ 0";
    } else if (timeout < SETTING_LIMITS.stay_timeout_minutes.min || timeout > SETTING_LIMITS.stay_timeout_minutes.max) {
      newErrors.stay_timeout_minutes = `ปรับได้ตั้งแต่ ${SETTING_LIMITS.stay_timeout_minutes.min} ถึง ${SETTING_LIMITS.stay_timeout_minutes.max} นาที`;
    }

    // 2. ตรวจสอบ Mount Height
    const height = Number(settings.mount_height);
    if (isNaN(height) || settings.mount_height === "" || height <= 0) {
      newErrors.mount_height = "ต้องเป็นตัวเลขบวก และห้ามติดลบหรือเท่ากับ 0";
    } else if (height < SETTING_LIMITS.mount_height.min || height > SETTING_LIMITS.mount_height.max) {
      newErrors.mount_height = `ความสูงต้องอยู่ระหว่าง ${SETTING_LIMITS.mount_height.min} ถึง ${SETTING_LIMITS.mount_height.max} เมตร`;
    }

    // 3. ตรวจสอบ Room Width
    const width = Number(settings.room_width);
    if (isNaN(width) || settings.room_width === "" || width <= 0) {
      newErrors.room_width = "ต้องเป็นตัวเลขบวก และห้ามติดลบหรือเท่ากับ 0";
    } else if (width < SETTING_LIMITS.room_width.min || width > SETTING_LIMITS.room_width.max) {
      newErrors.room_width = `ความกว้างต้องอยู่ระหว่าง ${SETTING_LIMITS.room_width.min} ถึง ${SETTING_LIMITS.room_width.max} เมตร`;
    }

    // 4. ตรวจสอบ Room Length
    const length = Number(settings.room_length);
    if (isNaN(length) || settings.room_length === "" || length <= 0) {
      newErrors.room_length = "ต้องเป็นตัวเลขบวก และห้ามติดลบหรือเท่ากับ 0";
    } else if (length < SETTING_LIMITS.room_length.min || length > SETTING_LIMITS.room_length.max) {
      newErrors.room_length = `ความยาวต้องอยู่ระหว่าง ${SETTING_LIMITS.room_length.min} ถึง ${SETTING_LIMITS.room_length.max} เมตร`;
    }

    // 5. ตรวจสอบ Fall Threshold
    const threshold = Number(settings.fall_threshold);
    if (isNaN(threshold) || settings.fall_threshold === "" || threshold <= 0) {
      newErrors.fall_threshold = "ต้องเป็นตัวเลขบวก และห้ามติดลบหรือเท่ากับ 0";
    } else if (threshold < SETTING_LIMITS.fall_threshold.min || threshold > SETTING_LIMITS.fall_threshold.max) {
      newErrors.fall_threshold = `เกณฑ์ตรวจจับต้องอยู่ระหว่าง ${SETTING_LIMITS.fall_threshold.min} ถึง ${SETTING_LIMITS.fall_threshold.max}`;
    }

    setErrors(newErrors);
  }, [settings, show]);

  if (!show) return null;

  // ป้องกันการพิมพ์เครื่องหมายลบ และตัวอักษรผิดประเภท
  const handleKeyDown = (e: React.KeyboardEvent<HTMLInputElement>) => {
    if (e.key === '-' || e.key === '+' || e.key === 'e' || e.key === 'E') {
      e.preventDefault();
    }
  };

  const handleNumberChange = (field: string, valStr: string) => {
    // ป้องกันค่าติดลบ
    if (valStr.startsWith('-')) return;

    setSettings({
      ...settings,
      [field]: valStr === "" ? "" : parseFloat(valStr)
    });
  };

  const safeVal = (val: any) => (Number.isNaN(val) ? "" : (val ?? ""));

  // ค่าพรีเซ็ตสำเร็จรูป (อิงตามระยะครอบคลุมจริงของเรดาร์ HLK-LD6002C)
  const applyPreset = (preset: 'standard' | 'elderly' | 'compact') => {
    if (preset === 'standard') {
      setSettings({
        ...settings,
        room_width: 2.0,
        room_length: 2.0,
        mount_height: 2.2,
        stay_timeout_minutes: 30.0,
        fall_threshold: 0.5,
      });
    } else if (preset === 'elderly') {
      setSettings({
        ...settings,
        room_width: 2.5,
        room_length: 2.5,
        mount_height: 2.2,
        stay_timeout_minutes: 20.0,
        fall_threshold: 0.45,
      });
    } else if (preset === 'compact') {
      setSettings({
        ...settings,
        room_width: 1.5,
        room_length: 1.5,
        mount_height: 2.2,
        stay_timeout_minutes: 20.0,
        fall_threshold: 0.5,
      });
    }
  };

  const hasErrors = Object.keys(errors).length > 0;

  return (
    <div className="fixed inset-0 bg-slate-900/60 backdrop-blur-xs flex items-center justify-center z-50 p-3 sm:p-4 overflow-y-auto">
      <div className="bg-white rounded-3xl w-full max-w-lg shadow-2xl border border-slate-200 overflow-hidden my-auto animate-in fade-in zoom-in-95 duration-150">
        
        {/* Header ของ Modal */}
        <div className="bg-gradient-to-r from-blue-600 via-indigo-600 to-blue-700 px-6 py-5 text-white flex items-center justify-between">
          <div className="flex items-center gap-3">
            <div className="w-10 h-10 rounded-xl bg-white/15 backdrop-blur-md flex items-center justify-center border border-white/20">
              <Settings2 size={22} className="text-white" />
            </div>
            <div>
              <h3 className="font-bold text-lg leading-tight">ตั้งค่าพารามิเตอร์เซนเซอร์</h3>
              <p className="text-blue-100 text-xs mt-0.5 font-mono">
                อุปกรณ์: <span className="font-semibold text-white bg-blue-500/40 px-2 py-0.5 rounded-md">{settings.device_id || '-'}</span>
              </p>
            </div>
          </div>
          <button 
            type="button" 
            onClick={onClose}
            className="w-8 h-8 rounded-full bg-white/10 hover:bg-white/20 flex items-center justify-center transition text-white"
            title="ปิดหน้าต่าง"
          >
            <X size={18} />
          </button>
        </div>

        <form onSubmit={onSubmit} className="p-5 sm:p-6 space-y-5 max-h-[80vh] overflow-y-auto">
          
          {/* พรีเซ็ตแนะนำสำหรับใช้งานด่วน */}
          <div className="bg-slate-50 p-3.5 rounded-2xl border border-slate-200/80">
            <div className="flex items-center gap-1.5 text-xs font-bold text-slate-700 mb-2">
              <Sparkles size={14} className="text-amber-500" />
              <span>พรีเซ็ตแนะนำ (คลิกเพื่อใส่ค่าอัตโนมัติ):</span>
            </div>
            <div className="grid grid-cols-3 gap-2">
              <button
                type="button"
                onClick={() => applyPreset('standard')}
                className="py-1.5 px-2 text-[11px] font-bold bg-blue-50 text-blue-700 border border-blue-200 rounded-xl transition text-center hover:bg-blue-100 shadow-2xs"
              >
                ⭐ ค่าแนะนำ (2×2ม. สูง 2.2ม.)
              </button>
              <button
                type="button"
                onClick={() => applyPreset('compact')}
                className="py-1.5 px-2 text-[11px] font-semibold bg-white hover:bg-slate-100 border border-slate-200 rounded-xl transition text-center text-slate-600 shadow-2xs"
              >
                ห้องน้ำเล็ก (1.5×1.5ม.)
              </button>
              <button
                type="button"
                onClick={() => applyPreset('elderly')}
                className="py-1.5 px-2 text-[11px] font-semibold bg-white hover:bg-slate-100 border border-slate-200 rounded-xl transition text-center text-slate-600 shadow-2xs"
              >
                ห้องน้ำกว้าง (2.5×2.5ม.)
              </button>
            </div>
          </div>

          {/* หมวดที่ 1: ขอบเขตและระยะติดตั้ง */}
          <div className="space-y-3">
            <div className="flex items-center gap-2 pb-1 border-b border-slate-100">
              <Maximize2 size={16} className="text-blue-600" />
              <h4 className="text-xs font-bold text-slate-800 uppercase tracking-wider">
                1. ขอบเขตพื้นที่ห้องน้ำและตำแหน่งติดตั้ง
              </h4>
            </div>

            <div className="grid grid-cols-1 sm:grid-cols-2 gap-3">
              {/* ความกว้างห้อง */}
              <div>
                <div className="flex items-center justify-between mb-1">
                  <label className="text-xs font-bold text-slate-700">ความกว้างห้อง (Width)</label>
                  <span className="text-[10px] text-slate-400 font-mono">0.8 - 4.0 ม. (แนะนำ 2.0ม.)</span>
                </div>
                <div className="relative">
                  <input
                    type="number"
                    step="0.1"
                    min="0.8"
                    max="4.0"
                    onKeyDown={handleKeyDown}
                    value={safeVal(settings.room_width)}
                    onChange={(e) => handleNumberChange('room_width', e.target.value)}
                    className={`w-full text-xs font-medium border rounded-xl p-2.5 pr-12 focus:ring-2 focus:outline-none transition ${
                      errors.room_width ? 'border-red-400 bg-red-50/30 focus:ring-red-300' : 'border-slate-300 focus:ring-blue-500'
                    }`}
                    placeholder="2.0"
                    required
                  />
                  <span className="absolute right-3 top-1/2 -translate-y-1/2 text-xs font-bold text-slate-400 pointer-events-none">
                    เมตร
                  </span>
                </div>
                {errors.room_width && (
                  <p className="text-[10px] text-red-500 mt-1 flex items-center gap-1 font-medium">
                    <AlertCircle size={10} /> {errors.room_width}
                  </p>
                )}
              </div>

              {/* ความยาวห้อง */}
              <div>
                <div className="flex items-center justify-between mb-1">
                  <label className="text-xs font-bold text-slate-700">ความยาวห้อง (Length)</label>
                  <span className="text-[10px] text-slate-400 font-mono">0.8 - 4.0 ม. (แนะนำ 2.0ม.)</span>
                </div>
                <div className="relative">
                  <input
                    type="number"
                    step="0.1"
                    min="0.8"
                    max="4.0"
                    onKeyDown={handleKeyDown}
                    value={safeVal(settings.room_length)}
                    onChange={(e) => handleNumberChange('room_length', e.target.value)}
                    className={`w-full text-xs font-medium border rounded-xl p-2.5 pr-12 focus:ring-2 focus:outline-none transition ${
                      errors.room_length ? 'border-red-400 bg-red-50/30 focus:ring-red-300' : 'border-slate-300 focus:ring-blue-500'
                    }`}
                    placeholder="2.0"
                    required
                  />
                  <span className="absolute right-3 top-1/2 -translate-y-1/2 text-xs font-bold text-slate-400 pointer-events-none">
                    เมตร
                  </span>
                </div>
                {errors.room_length && (
                  <p className="text-[10px] text-red-500 mt-1 flex items-center gap-1 font-medium">
                    <AlertCircle size={10} /> {errors.room_length}
                  </p>
                )}
              </div>
            </div>

            {/* ความสูงติดตั้งจากพื้น */}
            <div>
              <div className="flex items-center justify-between mb-1">
                <label className="text-xs font-bold text-slate-700 flex items-center gap-1">
                  <Ruler size={13} className="text-blue-500" />
                  <span>ความสูงติดตั้งจากพื้น (Mount Height)</span>
                </label>
                <span className="text-[10px] text-slate-400 font-mono">2.0 - 3.0 ม. (แนะนำ 2.2ม.)</span>
              </div>
              <div className="relative">
                <input
                  type="number"
                  step="0.1"
                  min="2.0"
                  max="3.0"
                  onKeyDown={handleKeyDown}
                  value={safeVal(settings.mount_height)}
                  onChange={(e) => handleNumberChange('mount_height', e.target.value)}
                  className={`w-full text-xs font-medium border rounded-xl p-2.5 pr-12 focus:ring-2 focus:outline-none transition ${
                    errors.mount_height ? 'border-red-400 bg-red-50/30 focus:ring-red-300' : 'border-slate-300 focus:ring-blue-500'
                  }`}
                  placeholder="2.2"
                  required
                />
                <span className="absolute right-3 top-1/2 -translate-y-1/2 text-xs font-bold text-slate-400 pointer-events-none">
                  เมตร
                </span>
              </div>
              {errors.mount_height ? (
                <p className="text-[10px] text-red-500 mt-1 flex items-center gap-1 font-medium">
                  <AlertCircle size={10} /> {errors.mount_height}
                </p>
              ) : (
                <p className="text-[10px] text-slate-400 mt-1">
                  * ค่าแนะนำ 2.2ม. ติดตั้งกึ่งกลางเพดาน รัศมีคลื่นรูปกรวย R=2.0ม. ครอบคลุมพื้นที่ 2×2ม.
                </p>
              )}
            </div>
          </div>

          {/* หมวดที่ 2: เกณฑ์ตรวจจับและความปลอดภัย */}
          <div className="space-y-3 pt-2">
            <div className="flex items-center gap-2 pb-1 border-b border-slate-100">
              <ShieldAlert size={16} className="text-indigo-600" />
              <h4 className="text-xs font-bold text-slate-800 uppercase tracking-wider">
                2. เกณฑ์ความปลอดภัยและการแจ้งเตือน
              </h4>
            </div>

            {/* แจ้งเตือนอยู่นานเกินกำหนด */}
            <div>
              <div className="flex items-center justify-between mb-1">
                <label className="text-xs font-bold text-slate-700 flex items-center gap-1">
                  <Clock size={13} className="text-indigo-500" />
                  <span>ระยะเวลาแจ้งเตือนอยู่นานเกินกำหนด (Stay Timeout)</span>
                </label>
                <span className="text-[10px] text-slate-400 font-mono">1.0 - 60.0 นาที</span>
              </div>
              <div className="relative">
                <input
                  type="number"
                  step="1.0"
                  min="1.0"
                  max="60.0"
                  onKeyDown={handleKeyDown}
                  value={safeVal(settings.stay_timeout_minutes)}
                  onChange={(e) => handleNumberChange('stay_timeout_minutes', e.target.value)}
                  className={`w-full text-xs font-medium border rounded-xl p-2.5 pr-14 focus:ring-2 focus:outline-none transition ${
                    errors.stay_timeout_minutes ? 'border-red-400 bg-red-50/30 focus:ring-red-300' : 'border-slate-300 focus:ring-blue-500'
                  }`}
                  placeholder="30"
                  required
                />
                <span className="absolute right-3 top-1/2 -translate-y-1/2 text-xs font-bold text-slate-400 pointer-events-none">
                  นาที
                </span>
              </div>
              {errors.stay_timeout_minutes ? (
                <p className="text-[10px] text-red-500 mt-1 flex items-center gap-1 font-medium">
                  <AlertCircle size={10} /> {errors.stay_timeout_minutes}
                </p>
              ) : (
                <p className="text-[10px] text-slate-400 mt-1">
                  * หากเซนเซอร์ LD2410C ตรวจพบบุคคลอยู่ในห้องน้ำเกินเวลานี้ จะส่งแจ้งเตือนทาง LINE ทันที
                </p>
              )}
            </div>

            {/* เกณฑ์ความไวการล้ม */}
            <div>
              <div className="flex items-center justify-between mb-1">
                <label className="text-xs font-bold text-slate-700 flex items-center gap-1">
                  <ArrowUpDown size={13} className="text-indigo-500" />
                  <span>เกณฑ์ความสูงตัดสินการล้ม (Fall Threshold)</span>
                </label>
                <span className="text-[10px] text-slate-400 font-mono">0.20 - 0.80 ม. (ค่ามาตรฐาน 0.50ม.)</span>
              </div>
              <div className="relative">
                <input
                  type="number"
                  step="0.05"
                  min="0.2"
                  max="0.8"
                  onKeyDown={handleKeyDown}
                  value={safeVal(settings.fall_threshold)}
                  onChange={(e) => handleNumberChange('fall_threshold', e.target.value)}
                  className={`w-full text-xs font-medium border rounded-xl p-2.5 pr-12 focus:ring-2 focus:outline-none transition ${
                    errors.fall_threshold ? 'border-red-400 bg-red-50/30 focus:ring-red-300' : 'border-slate-300 focus:ring-blue-500'
                  }`}
                  placeholder="0.5"
                  required
                />
                <span className="absolute right-3 top-1/2 -translate-y-1/2 text-xs font-bold text-slate-400 pointer-events-none">
                  เมตร
                </span>
              </div>
              {errors.fall_threshold ? (
                <p className="text-[10px] text-red-500 mt-1 flex items-center gap-1 font-medium">
                  <AlertCircle size={10} /> {errors.fall_threshold}
                </p>
              ) : (
                <p className="text-[10px] text-slate-400 mt-1">
                  * ระดับความสูงของ Point Cloud จากพื้น (0.50ม. ป้องกัน False Alarm ขณะนั่งชักโครกหรือก้ม)
                </p>
              )}
            </div>
          </div>

          {/* ข้อความเตือนรวมหากมีช่องที่ผิด */}
          {hasErrors && (
            <div className="p-3 bg-red-50 border border-red-200 rounded-xl text-xs text-red-600 flex items-center gap-2">
              <AlertCircle size={16} className="flex-shrink-0" />
              <span>มีบางช่องที่ค่าไม่ถูกต้อง หรือติดลบ โปรดแก้ไขก่อนกดยืนยัน</span>
            </div>
          )}

          {/* ปุ่มบันทึกและยกเลิก */}
          <div className="flex items-center justify-end gap-2.5 pt-3 border-t border-slate-100">
            <button
              type="button"
              onClick={onClose}
              className="px-4 py-2.5 rounded-xl text-xs font-medium text-slate-600 hover:bg-slate-100 transition"
            >
              ยกเลิก
            </button>
            <button
              type="submit"
              disabled={hasErrors}
              className={`px-5 py-2.5 rounded-xl text-xs font-bold transition shadow-md flex items-center gap-1.5 ${
                hasErrors 
                  ? 'bg-slate-300 text-slate-500 cursor-not-allowed shadow-none' 
                  : 'bg-gradient-to-r from-blue-600 to-indigo-600 hover:from-blue-700 hover:to-indigo-700 text-white'
              }`}
            >
              <Check size={15} />
              <span>บันทึกพารามิเตอร์</span>
            </button>
          </div>

        </form>
      </div>
    </div>
  );
}