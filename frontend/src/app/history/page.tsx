"use client";

import { useState, useEffect, useCallback } from 'react';
import Header from '@/components/Dashboard/Header';
import { eventService, groupService } from '@/services/api';
import { 
  AlertTriangle, 
  CheckCircle, 
  Clock, 
  Download, 
  RefreshCw, 
  Search, 
  X, 
  Filter,
  CheckCircle2,
  AlertCircle
} from 'lucide-react';
import { useRouter } from 'next/navigation';

export default function HistoryPage() {
  const router = useRouter();

  const [events, setEvents] = useState<any[]>([]);
  const [total, setTotal] = useState<number>(0);
  const [stats, setStats] = useState<{
    total_falls: number;
    pending_falls: number;
    resolved_falls: number;
    online_devices: number;
    total_devices: number;
  }>({
    total_falls: 0,
    pending_falls: 0,
    resolved_falls: 0,
    online_devices: 0,
    total_devices: 0,
  });

  const [groups, setGroups] = useState<any[]>([]);
  const [loading, setLoading] = useState<boolean>(true);

  // Filters State
  const [filterType, setFilterType] = useState<string>("fall"); // เริ่มต้นที่การล้ม
  const [filterGroup, setFilterGroup] = useState<string>("all");
  const [filterStatus, setFilterStatus] = useState<string>("all");
  const [search, setSearch] = useState<string>("");

  // Alert State
  const [alertMsg, setAlertMsg] = useState<string | null>(null);

  // Modal จัดการช่วยเหลือ (Resolve Modal)
  const [resolveModal, setResolveModal] = useState<{
    show: boolean;
    event: any | null;
  }>({ show: false, event: null });
  const [resolveNote, setResolveNote] = useState<string>("");
  const [resolveAction, setResolveAction] = useState<string>("resolved"); // "resolved" หรือ "false_alarm"
  const [isSubmitting, setIsSubmitting] = useState<boolean>(false);

  // ดึงข้อมูลกลุ่มสถานที่
  const fetchGroups = async () => {
    try {
      const res = await groupService.getAll();
      setGroups(res.data || []);
    } catch (err) {
      console.error("ดึงข้อมูลกลุ่มไม่สำเร็จ:", err);
    }
  };

  // ดึงรายการประวัติเหตุการณ์
  const fetchEvents = useCallback(async () => {
    try {
      setLoading(true);
      const params: any = { limit: 100 };
      if (filterType !== 'all') params.name = filterType;
      if (filterGroup !== 'all') params.group_id = filterGroup;
      if (filterStatus !== 'all') params.status = filterStatus;
      if (search.trim()) params.search = search.trim();

      const res = await eventService.getAll(params);
      setEvents(res.data.events || []);
      setTotal(res.data.total || 0);
    } catch (err) {
      console.error("ดึงข้อมูลประวัติไม่สำเร็จ:", err);
    } finally {
      setLoading(false);
    }
  }, [filterType, filterGroup, filterStatus, search]);

  // ดึงสถิติภาพรวม
  const fetchStats = useCallback(async () => {
    try {
      const gId = filterGroup !== 'all' ? parseInt(filterGroup, 10) : undefined;
      const res = await eventService.getStats(gId);
      setStats(res.data);
    } catch (err) {
      console.error("ดึงสถิติไม่สำเร็จ:", err);
    }
  }, [filterGroup]);

  // โหลดข้อมูลทั้งหมด
  const refreshAll = useCallback(() => {
    fetchEvents();
    fetchStats();
  }, [fetchEvents, fetchStats]);

  useEffect(() => {
    const token = typeof window !== 'undefined' ? localStorage.getItem('token') : null;
    if (!token) {
      router.push('/login');
      return;
    }

    fetchGroups();
    refreshAll();

    // เชื่อมต่อ WebSocket สำหรับการแจ้งเตือน Realtime
    const wsProtocol = window.location.protocol === 'https:' ? 'wss:' : 'ws:';
    const wsHost = window.location.host;
    const wsUrl = (window.location.hostname === 'localhost' || window.location.hostname === '127.0.0.1')
      ? 'ws://localhost:8080/ws'
      : `${wsProtocol}//${wsHost}/ws`;
    
    let ws: WebSocket | null = null;
    try {
      ws = new WebSocket(wsUrl);
      ws.onmessage = (event) => {
        try {
          const data = JSON.parse(event.data);
          refreshAll();
          if (data.name === 'fall') {
            setAlertMsg(`🚨 มีการตรวจพบเหตุล้มใหม่! รหัสอุปกรณ์: ${data.device_id || 'เซนเซอร์'}`);
          }
        } catch (e) {
          // ignore
        }
      };
    } catch (e) {
      console.warn("WebSocket ไม่พร้อมใช้งาน");
    }

    return () => {
      if (ws) ws.close();
    };
  }, [router, refreshAll]);

  // ฟอร์แมตวันเวลาแบบไทย
  const formatDateTime = (isoStr: string) => {
    if (!isoStr) return '-';
    const d = new Date(isoStr);
    return d.toLocaleDateString('th-TH', { 
      day: '2-digit', 
      month: 'short', 
      year: 'numeric' 
    }) + ' เวลา ' + d.toLocaleTimeString('th-TH', { 
      hour: '2-digit', 
      minute: '2-digit'
    }) + ' น.';
  };

  // จัดการบันทึกสถานะการช่วยเหลือ
  const handleConfirmResolve = async () => {
    if (!resolveModal.event) return;
    try {
      setIsSubmitting(true);
      await eventService.resolve(resolveModal.event.id, {
        status: resolveAction,
        resolution_note: resolveNote || (resolveAction === 'resolved' ? "เข้าช่วยเหลือแล้ว ปลอดภัยดี" : "สัญญาณแจ้งเตือนผิดพลาด")
      });

      // ปิด modal และรีเฟรชข้อมูล
      setResolveModal({ show: false, event: null });
      setResolveNote("");
      refreshAll();
      setAlertMsg(null);
    } catch (err: any) {
      alert("บันทึกไม่สำเร็จ: " + (err.response?.data?.error || err.message));
    } finally {
      setIsSubmitting(false);
    }
  };

  // ฟังก์ชันแปลงชื่อเหตุการณ์เป็นภาษาไทย
  const getEventNameText = (name: string) => {
    switch (name) {
      case 'fall': return 'ตรวจพบการล้ม';
      case 'prolonged_stay': return 'อยู่นานเกินกำหนด';
      case 'enter': return 'มีคนเข้าห้อง';
      case 'exit': return 'คนออกจากห้อง';
      case 'online': return 'อุปกรณ์ออนไลน์';
      case 'offline': return 'อุปกรณ์ออฟไลน์';
      default: return name;
    }
  };

  // ส่งออกข้อมูลเป็น CSV
  const handleExportCSV = () => {
    if (events.length === 0) {
      alert("ไม่มีข้อมูลสำหรับส่งออก");
      return;
    }
    let csv = "\uFEFFลำดับ,วันเวลา,สถานที่,ห้อง/จุดติดตั้ง,รหัสอุปกรณ์,ประเภทเหตุการณ์,สถานะการช่วยเหลือ,ผู้เข้าช่วยเหลือ,เวลาที่เข้าช่วยเหลือ,บันทึก\n";
    events.forEach((e, idx) => {
      const statusText = e.status === 'resolved' ? 'ช่วยเหลือแล้ว' : (e.status === 'false_alarm' ? 'สัญญาณหลอก' : (e.status === 'pending' ? 'รอตรวจสอบ/ช่วยเหลือ' : 'ปกติ'));
      const eventText = getEventNameText(e.name);
      const resAt = e.resolved_at ? formatDateTime(e.resolved_at) : '-';
      csv += `${idx + 1},"${formatDateTime(e.created_at)}","${e.group_name || '-'}","${e.device_name || '-'}","${e.device_code || '-'}","${eventText}","${statusText}","${e.resolved_by || '-'}","${resAt}","${e.resolution_note || '-'}"\n`;
    });

    const blob = new Blob([csv], { type: 'text/csv;charset=utf-8;' });
    const url = URL.createObjectURL(blob);
    const a = document.createElement('a');
    a.href = url;
    a.download = `fall_detection_history_${new Date().toISOString().slice(0, 10)}.csv`;
    a.click();
  };

  return (
    <div className="p-4 sm:p-8 bg-gray-50 min-h-screen">
      {/* 1. Header ส่วนบน */}
      <Header />

      {/* แถบแจ้งเตือนฉุกเฉิน Live Alert */}
      {alertMsg && (
        <div className="mb-6 p-4 bg-red-600 text-white rounded-2xl shadow-lg border border-red-700 flex justify-between items-center animate-pulse">
          <div className="flex items-center gap-3">
            <AlertTriangle className="flex-shrink-0 animate-bounce" size={24} />
            <span className="font-bold text-sm sm:text-base">{alertMsg}</span>
          </div>
          <button 
            onClick={() => setAlertMsg(null)}
            className="p-1 hover:bg-red-700 rounded-lg transition"
          >
            <X size={20} />
          </button>
        </div>
      )}

      {/* 2. การ์ดสรุปสถิติ (KPI Summary) */}
      <div className="grid grid-cols-2 lg:grid-cols-4 gap-3 sm:gap-5 mb-6">
        <div className="bg-white p-4 sm:p-5 rounded-2xl border border-gray-200/80 shadow-xs flex flex-col justify-between">
          <div className="flex items-center justify-between text-gray-500 text-xs mb-1">
            <span>การล้มทั้งหมด</span>
            <span className="p-1.5 bg-red-50 text-red-600 rounded-lg font-bold">🚨</span>
          </div>
          <div className="flex items-baseline gap-2">
            <span className="text-2xl sm:text-3xl font-extrabold text-gray-800">{stats.total_falls}</span>
            <span className="text-xs text-gray-400">ครั้ง</span>
          </div>
          <span className="text-[11px] text-gray-400 mt-2">บันทึกทั้งหมดในระบบ</span>
        </div>

        <div className="bg-white p-4 sm:p-5 rounded-2xl border border-gray-200/80 shadow-xs flex flex-col justify-between">
          <div className="flex items-center justify-between text-gray-500 text-xs mb-1">
            <span>รอการช่วยเหลือ (Pending)</span>
            <span className="p-1.5 bg-red-50 text-red-600 rounded-lg font-bold animate-pulse">⏳</span>
          </div>
          <div className="flex items-baseline gap-2">
            <span className="text-2xl sm:text-3xl font-extrabold text-red-600">{stats.pending_falls}</span>
            <span className="text-xs text-red-500 font-semibold">เคสฉุกเฉิน</span>
          </div>
          <span className="text-[11px] text-red-400 mt-2">ต้องการการเข้าดูแลทันที</span>
        </div>

        <div className="bg-white p-4 sm:p-5 rounded-2xl border border-gray-200/80 shadow-xs flex flex-col justify-between">
          <div className="flex items-center justify-between text-gray-500 text-xs mb-1">
            <span>ช่วยเหลือเรียบร้อย</span>
            <span className="p-1.5 bg-emerald-50 text-emerald-600 rounded-lg font-bold">✅</span>
          </div>
          <div className="flex items-baseline gap-2">
            <span className="text-2xl sm:text-3xl font-extrabold text-emerald-600">{stats.resolved_falls}</span>
            <span className="text-xs text-gray-400">ครั้ง</span>
          </div>
          <span className="text-[11px] text-emerald-600 mt-2">ปลอดภัยทุกเหตุการณ์</span>
        </div>

        <div className="bg-white p-4 sm:p-5 rounded-2xl border border-gray-200/80 shadow-xs flex flex-col justify-between">
          <div className="flex items-center justify-between text-gray-500 text-xs mb-1">
            <span>อุปกรณ์ที่เฝ้าระวัง</span>
            <span className="p-1.5 bg-blue-50 text-blue-600 rounded-lg font-bold">📡</span>
          </div>
          <div className="flex items-baseline gap-2">
            <span className="text-2xl sm:text-3xl font-extrabold text-blue-600">
              {stats.online_devices} / {stats.total_devices}
            </span>
            <span className="text-xs text-emerald-600 font-medium">ออนไลน์</span>
          </div>
          <span className="text-[11px] text-gray-400 mt-2">พร้อมตรวจจับตลอด 24 ชม.</span>
        </div>
      </div>

      {/* 3. ส่วนของตารางและตัวกรองข้อมูล */}
      <div className="bg-white rounded-2xl border border-gray-200 shadow-sm p-4 sm:p-6 space-y-4">
        {/* หัวข้อตารางและปุ่มเครื่องมือ */}
        <div className="flex flex-col md:flex-row md:items-center justify-between gap-3 border-b border-gray-100 pb-4">
          <div>
            <h2 className="text-lg font-bold text-gray-800 flex items-center gap-2">
              <span>บันทึกประวัติเหตุการณ์ (Activity & Incident Logs)</span>
              <span className="text-xs font-normal text-gray-400">(พบ {total} รายการ)</span>
            </h2>
            <p className="text-xs text-gray-400">บันทึกวันเวลา สถานที่ และการเข้าช่วยเหลือผู้ป่วย/ผู้สูงอายุ</p>
          </div>

          <div className="flex flex-wrap items-center gap-2">
            <button 
              onClick={handleExportCSV} 
              className="px-3 py-1.5 text-xs font-medium text-gray-700 bg-gray-50 hover:bg-gray-100 border border-gray-200 rounded-xl transition flex items-center gap-1.5"
            >
              <Download size={14} />
              <span>Export CSV</span>
            </button>
            <button 
              onClick={refreshAll} 
              className="px-3 py-1.5 text-xs font-medium text-blue-600 bg-blue-50 hover:bg-blue-100 border border-blue-200 rounded-xl transition flex items-center gap-1.5"
            >
              <RefreshCw size={14} className={loading ? "animate-spin" : ""} />
              <span>รีเฟรช</span>
            </button>
          </div>
        </div>

        {/* แถบตัวกรอง (Filters Bar) */}
        <div className="grid grid-cols-1 sm:grid-cols-2 lg:grid-cols-4 gap-3 bg-gray-50 p-3.5 rounded-xl border border-gray-200/80">
          <div>
            <label className="block text-[11px] font-bold text-gray-600 mb-1 flex items-center gap-1">
              <Filter size={11} /> ประเภทเหตุการณ์
            </label>
            <select 
              value={filterType} 
              onChange={(e) => setFilterType(e.target.value)} 
              className="w-full text-xs bg-white border border-gray-300 rounded-lg p-2 focus:ring-2 focus:ring-blue-500 focus:outline-none"
            >
              <option value="fall">🚨 เฉพาะการล้ม (Falls Only)</option>
              <option value="prolonged_stay">⏳ อยู่นานเกินกำหนด (Stay Timeout)</option>
              <option value="all">ทั้งหมด (All Events)</option>
              <option value="enter">🚶 มีคนเข้าห้อง (Enter)</option>
              <option value="exit">🚪 คนออกจากห้อง (Exit)</option>
              <option value="online">🟢 อุปกรณ์ออนไลน์ (Online)</option>
              <option value="offline">⚪ อุปกรณ์ออฟไลน์ (Offline)</option>
            </select>
          </div>

          <div>
            <label className="block text-[11px] font-bold text-gray-600 mb-1">กลุ่ม / สถานที่</label>
            <select 
              value={filterGroup} 
              onChange={(e) => setFilterGroup(e.target.value)} 
              className="w-full text-xs bg-white border border-gray-300 rounded-lg p-2 focus:ring-2 focus:ring-blue-500 focus:outline-none"
            >
              <option value="all">ทุกสถานที่ (All Groups)</option>
              {groups.map((g) => (
                <option key={g.ID} value={g.ID}>{g.name}</option>
              ))}
            </select>
          </div>

          <div>
            <label className="block text-[11px] font-bold text-gray-600 mb-1">สถานะการช่วยเหลือ</label>
            <select 
              value={filterStatus} 
              onChange={(e) => setFilterStatus(e.target.value)} 
              className="w-full text-xs bg-white border border-gray-300 rounded-lg p-2 focus:ring-2 focus:ring-blue-500 focus:outline-none"
            >
              <option value="all">ทุกสถานะ (All Status)</option>
              <option value="pending">⏳ ยังไม่เข้าช่วยเหลือ (Pending)</option>
              <option value="resolved">✅ ช่วยเหลือแล้ว (Resolved)</option>
              <option value="false_alarm">⚠️ สัญญาณเตือนหลอก (False Alarm)</option>
            </select>
          </div>

          <div>
            <label className="block text-[11px] font-bold text-gray-600 mb-1 flex items-center gap-1">
              <Search size={11} /> ค้นหาห้อง / อุปกรณ์
            </label>
            <input 
              type="text" 
              value={search} 
              onChange={(e) => setSearch(e.target.value)} 
              placeholder="เช่น ห้องนอน 101..." 
              className="w-full text-xs bg-white border border-gray-300 rounded-lg p-2 focus:ring-2 focus:ring-blue-500 focus:outline-none"
            />
          </div>
        </div>

        {/* ตารางข้อมูลเหตุการณ์ */}
        <div className="overflow-x-auto border border-gray-100 rounded-xl">
          <table className="w-full text-left text-xs text-gray-600">
            <thead className="bg-gray-100 text-gray-700 font-semibold uppercase text-[10px] tracking-wider border-b border-gray-200">
              <tr>
                <th className="py-3 px-4">วันและเวลาที่เกิดเหตุ</th>
                <th className="py-3 px-4">สถานที่ & จุดติดตั้ง</th>
                <th className="py-3 px-4">เหตุการณ์</th>
                <th className="py-3 px-4">สถานะช่วยเหลือ</th>
                <th className="py-3 px-4">ผู้เข้าช่วยเหลือ / บันทึก</th>
                <th className="py-3 px-4 text-center">จัดการเหตุการณ์</th>
              </tr>
            </thead>
            <tbody className="divide-y divide-gray-100">
              {loading ? (
                <tr>
                  <td colSpan={6} className="py-8 text-center text-gray-400">
                    <RefreshCw className="animate-spin inline-block mr-2" size={18} />
                    กำลังโหลดข้อมูล...
                  </td>
                </tr>
              ) : events.length === 0 ? (
                <tr>
                  <td colSpan={6} className="py-8 text-center text-gray-400">
                    <div className="text-3xl mb-1">🔍</div>
                    <div>ไม่พบรายการเหตุการณ์ตามเงื่อนไขที่เลือก</div>
                  </td>
                </tr>
              ) : (
                events.map((ev) => {
                  const isFall = ev.name === 'fall';
                  const isProlongedStay = ev.name === 'prolonged_stay';
                  const isAlert = isFall || isProlongedStay;
                  const isPending = isAlert && ev.status === 'pending';

                  return (
                    <tr 
                      key={ev.id} 
                      className={`hover:bg-gray-50/80 transition-colors ${
                        isPending 
                          ? (isFall ? 'bg-red-50/40' : 'bg-orange-50/40') 
                          : ''
                      }`}
                    >
                      {/* วันและเวลา */}
                      <td className="py-3 px-4 whitespace-nowrap">
                        <div className="font-bold text-gray-800">{formatDateTime(ev.created_at)}</div>
                        <div className="text-[10px] text-gray-400 font-mono">Event #{ev.id}</div>
                      </td>

                      {/* สถานที่และห้อง */}
                      <td className="py-3 px-4">
                        <div className="font-bold text-gray-800">{ev.device_name || 'ไม่ระบุห้อง'}</div>
                        <div className="text-[10px] text-gray-500 flex items-center gap-1">
                          <span>🏠 {ev.group_name || 'ทั่วไป'}</span>
                          <span>•</span>
                          <span className="font-mono text-gray-400">{ev.device_code}</span>
                        </div>
                      </td>

                      {/* ประเภทเหตุการณ์ */}
                      <td className="py-3 px-4">
                        {isFall ? (
                          <span className="inline-flex items-center gap-1.5 px-2.5 py-1 rounded-full text-[11px] font-bold bg-red-100 text-red-700 border border-red-200">
                            <span className="w-2 h-2 rounded-full bg-red-600 animate-pulse"></span>
                            ตรวจพบการล้ม
                          </span>
                        ) : isProlongedStay ? (
                          <span className="inline-flex items-center gap-1.5 px-2.5 py-1 rounded-full text-[11px] font-bold bg-orange-100 text-orange-700 border border-orange-200">
                            <Clock size={12} className="text-orange-500" />
                            อยู่นานเกินกำหนด
                          </span>
                        ) : ev.name === 'enter' ? (
                          <span className="inline-flex items-center gap-1 px-2.5 py-1 rounded-full text-[11px] font-medium bg-blue-100 text-blue-700 border border-blue-200">
                            🚶 มีคนเข้าห้อง
                          </span>
                        ) : ev.name === 'exit' ? (
                          <span className="inline-flex items-center gap-1 px-2.5 py-1 rounded-full text-[11px] font-medium bg-gray-100 text-gray-700 border border-gray-200">
                            🚪 คนออกจากห้อง
                          </span>
                        ) : ev.name === 'online' ? (
                          <span className="inline-flex items-center gap-1 px-2.5 py-1 rounded-full text-[11px] font-medium bg-emerald-50 text-emerald-700 border border-emerald-200">
                            <span className="w-1.5 h-1.5 rounded-full bg-emerald-500"></span>
                            🟢 อุปกรณ์ออนไลน์
                          </span>
                        ) : ev.name === 'offline' ? (
                          <span className="inline-flex items-center gap-1 px-2.5 py-1 rounded-full text-[11px] font-medium bg-rose-50 text-rose-600 border border-rose-200">
                            <span className="w-1.5 h-1.5 rounded-full bg-rose-500"></span>
                            ⚪ อุปกรณ์ออฟไลน์
                          </span>
                        ) : (
                          <span className="inline-flex items-center gap-1 px-2.5 py-1 rounded-full text-[11px] font-medium bg-gray-100 text-gray-600 border border-gray-200">
                            {ev.value || ev.name}
                          </span>
                        )}
                      </td>

                      {/* สถานะช่วยเหลือ */}
                      <td className="py-3 px-4">
                        {isAlert ? (
                          isPending ? (
                            <span className={`inline-flex items-center gap-1 px-2.5 py-1 rounded-lg text-[11px] font-bold text-white ${
                              isFall ? 'bg-red-600 animate-pulse' : 'bg-orange-500'
                            }`}>
                              <Clock size={12} /> {isFall ? 'ยังไม่เข้าช่วยเหลือ' : 'รอตรวจสอบ'}
                            </span>
                          ) : ev.status === 'resolved' ? (
                            <span className="inline-flex items-center gap-1 px-2.5 py-1 rounded-lg text-[11px] font-bold bg-emerald-100 text-emerald-700 border border-emerald-200">
                              <CheckCircle size={12} /> {isFall ? 'ช่วยเหลือแล้ว' : 'ตรวจสอบแล้ว'}
                            </span>
                          ) : (
                            <span className="inline-flex items-center gap-1 px-2.5 py-1 rounded-lg text-[11px] font-medium bg-amber-100 text-amber-700 border border-amber-200">
                              <AlertCircle size={12} /> สัญญาณหลอก
                            </span>
                          )
                        ) : (ev.name === 'online' || ev.name === 'offline') ? (
                          <span className="text-gray-400 text-[11px]">- สถานะระบบ -</span>
                        ) : (
                          <span className="text-gray-400 text-[11px]">- กิจกรรมปกติ -</span>
                        )}
                      </td>

                      {/* ผู้ช่วยเหลือ / บันทึก */}
                      <td className="py-3 px-4">
                        {ev.resolved_by ? (
                          <div>
                            <p className="font-bold text-gray-800 text-[11px]">👤 {ev.resolved_by}</p>
                            <p className="text-[10px] text-gray-500">{formatDateTime(ev.resolved_at)}</p>
                            {ev.resolution_note && (
                              <p className="text-[10px] text-emerald-600 mt-0.5 font-medium">&quot;{ev.resolution_note}&quot;</p>
                            )}
                          </div>
                        ) : isPending ? (
                          <span className={`${isFall ? 'text-red-500' : 'text-orange-500'} font-semibold text-[11px]`}>
                            {isFall ? 'รอเจ้าหน้าที่เข้าช่วยเหลือ' : 'รอเจ้าหน้าที่ตรวจสอบ'}
                          </span>
                        ) : (
                          <span className="text-gray-300 text-[10px]">-</span>
                        )}
                      </td>

                      {/* ปุ่มจัดการเหตุการณ์ */}
                      <td className="py-3 px-4 text-center">
                        {isPending ? (
                          <button 
                            onClick={() => {
                              setResolveAction("resolved");
                              setResolveNote("");
                              setResolveModal({ show: true, event: ev });
                            }} 
                            className="px-3 py-1.5 bg-emerald-600 hover:bg-emerald-700 text-white rounded-xl text-xs font-bold shadow-xs transition flex items-center gap-1 mx-auto"
                          >
                            <CheckCircle2 size={13} />
                            <span>{isFall ? 'ช่วยเหลือแล้ว' : 'ตรวจสอบแล้ว'}</span>
                          </button>
                        ) : isAlert ? (
                          <span className="text-[10px] text-gray-400">ปิดงานเรียบร้อย</span>
                        ) : (
                          <span className="text-gray-300 text-[10px]">-</span>
                        )}
                      </td>
                    </tr>
                  );
                })
              )}
            </tbody>
          </table>
        </div>
      </div>

      {/* 4. Modal สำหรับบันทึกการเข้าช่วยเหลือ */}
      {resolveModal.show && resolveModal.event && (
        <div className="fixed inset-0 bg-black/50 z-50 flex items-center justify-center p-4">
          <div className="bg-white rounded-2xl max-w-md w-full p-5 sm:p-6 shadow-xl border border-gray-200 animate-in fade-in zoom-in-95 duration-150">
            <div className="flex items-center justify-between border-b pb-3 mb-4">
              <h3 className="font-bold text-base text-gray-800 flex items-center gap-2">
                <span>บันทึกการเข้าช่วยเหลือ</span>
                <span className="text-xs bg-red-100 text-red-700 px-2 py-0.5 rounded-full font-mono">
                  #{resolveModal.event.id}
                </span>
              </h3>
              <button 
                onClick={() => setResolveModal({ show: false, event: null })} 
                className="text-gray-400 hover:text-gray-600 p-1"
              >
                <X size={18} />
              </button>
            </div>

            <div className="space-y-4">
              <div className="bg-gray-50 p-3 rounded-xl text-xs text-gray-600 space-y-1 border border-gray-200/60">
                <p><strong>สถานที่:</strong> {resolveModal.event.group_name || 'ทั่วไป'}</p>
                <p><strong>ห้อง/จุดติดตั้ง:</strong> {resolveModal.event.device_name} ({resolveModal.event.device_code})</p>
                <p><strong>เวลาเกิดเหตุ:</strong> {formatDateTime(resolveModal.event.created_at)}</p>
              </div>

              {/* เลือกผลการตรวจสอบ */}
              <div>
                <label className="block text-xs font-bold text-gray-700 mb-1.5">ผลการตรวจสอบเหตุการณ์</label>
                <div className="grid grid-cols-2 gap-2">
                  <button 
                    type="button" 
                    onClick={() => setResolveAction("resolved")} 
                    className={`py-2 px-3 rounded-xl border text-xs font-bold transition flex items-center justify-center gap-1.5 ${resolveAction === 'resolved' ? 'bg-emerald-50 border-emerald-500 text-emerald-700 shadow-xs' : 'bg-white border-gray-200 text-gray-600'}`}
                  >
                    <CheckCircle size={14} />
                    <span>เข้าช่วยเหลือแล้ว</span>
                  </button>
                  <button 
                    type="button" 
                    onClick={() => setResolveAction("false_alarm")} 
                    className={`py-2 px-3 rounded-xl border text-xs font-bold transition flex items-center justify-center gap-1.5 ${resolveAction === 'false_alarm' ? 'bg-amber-50 border-amber-500 text-amber-700 shadow-xs' : 'bg-white border-gray-200 text-gray-600'}`}
                  >
                    <AlertCircle size={14} />
                    <span>สัญญาณเตือนหลอก</span>
                  </button>
                </div>
              </div>

              {/* ช่องกรอกบันทึกเพิ่มเติม */}
              <div>
                <label className="block text-xs font-bold text-gray-700 mb-1">
                  บันทึกเพิ่มเติม (Note)
                </label>
                <textarea 
                  rows={3} 
                  value={resolveNote} 
                  onChange={(e) => setResolveNote(e.target.value)} 
                  placeholder={resolveAction === 'resolved' ? "เช่น พยุงลุกขึ้น ปฐมพยาบาลเบื้องต้นแล้วปลอดภัยดี..." : "เช่น วัตถุตกหล่น ไม่พบคนหกล้ม..."} 
                  className="w-full text-xs border border-gray-300 rounded-xl p-3 focus:ring-2 focus:ring-blue-500 focus:outline-none"
                />
              </div>

              {/* ปุ่มบันทึก */}
              <div className="flex gap-2 justify-end pt-2">
                <button 
                  type="button" 
                  onClick={() => setResolveModal({ show: false, event: null })} 
                  className="px-4 py-2 text-xs font-medium text-gray-600 hover:bg-gray-100 rounded-xl transition"
                >
                  ยกเลิก
                </button>
                <button 
                  type="button" 
                  disabled={isSubmitting} 
                  onClick={handleConfirmResolve} 
                  className="px-4 py-2 bg-blue-600 hover:bg-blue-700 text-white rounded-xl text-xs font-bold shadow transition disabled:opacity-50"
                >
                  {isSubmitting ? "กำลังบันทึก..." : "ยืนยันการบันทึก"}
                </button>
              </div>
            </div>
          </div>
        </div>
      )}

    </div>
  );
}
