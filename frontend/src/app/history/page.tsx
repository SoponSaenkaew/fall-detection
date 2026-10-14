"use client";

import { useState, useEffect, useCallback, useRef } from 'react';
import Header from '@/components/Dashboard/Header';
import { eventService, groupService } from '@/services/api';
import { 
  AlertTriangle, 
  Clock, 
  Download, 
  RefreshCw, 
  Search, 
  X, 
  Filter,
  ChevronLeft,
  ChevronRight,
  ChevronsLeft,
  ChevronsRight
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
  const [search, setSearch] = useState<string>("");

  // Pagination State
  const [page, setPage] = useState<number>(1);
  const [pageSize, setPageSize] = useState<string>("50"); // "50", "100", "250", "all"
  const [isExporting, setIsExporting] = useState<boolean>(false);

  const handleFilterTypeChange = (val: string) => {
    setFilterType(val);
    setPage(1);
  };
  const handleFilterGroupChange = (val: string) => {
    setFilterGroup(val);
    setPage(1);
  };
  const handleSearchChange = (val: string) => {
    setSearch(val);
    setPage(1);
  };
  const handlePageSizeChange = (val: string) => {
    setPageSize(val);
    setPage(1);
  };

  const totalPages = pageSize === 'all' ? 1 : Math.ceil(total / Number(pageSize)) || 1;

  const getPageNumbers = () => {
    const pages: (number | string)[] = [];
    if (totalPages <= 7) {
      for (let i = 1; i <= totalPages; i++) pages.push(i);
    } else {
      if (page <= 4) {
        for (let i = 1; i <= 5; i++) pages.push(i);
        pages.push('...');
        pages.push(totalPages);
      } else if (page >= totalPages - 3) {
        pages.push(1);
        pages.push('...');
        for (let i = totalPages - 4; i <= totalPages; i++) pages.push(i);
      } else {
        pages.push(1);
        pages.push('...');
        pages.push(page - 1);
        pages.push(page);
        pages.push(page + 1);
        pages.push('...');
        pages.push(totalPages);
      }
    }
    return pages;
  };

  // Alert State
  const [alertMsg, setAlertMsg] = useState<string | null>(null);

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
      const params: any = {};
      if (pageSize === 'all') {
        params.limit = 'all';
      } else {
        params.limit = parseInt(pageSize, 10);
        params.page = page;
      }
      if (filterType !== 'all') params.name = filterType;
      if (filterGroup !== 'all') params.group_id = filterGroup;
      if (search.trim()) params.search = search.trim();

      const res = await eventService.getAll(params);
      setEvents(res.data.events || []);
      setTotal(res.data.total || 0);
    } catch (err) {
      console.error("ดึงข้อมูลประวัติไม่สำเร็จ:", err);
    } finally {
      setLoading(false);
    }
  }, [filterType, filterGroup, search, page, pageSize]);

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

  const refreshAllRef = useRef(refreshAll);
  refreshAllRef.current = refreshAll;

  // ดึงรายการประวัติเมื่อตัวกรองเปลี่ยน
  useEffect(() => {
    fetchEvents();
  }, [fetchEvents]);

  // ดึงตัวเลขสถิติเมื่อเปลี่ยนสถานที่
  useEffect(() => {
    fetchStats();
  }, [fetchStats]);

  useEffect(() => {
    const token = typeof window !== 'undefined' ? localStorage.getItem('token') : null;
    if (!token) {
      router.push('/login');
      return;
    }

    fetchGroups();

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
          refreshAllRef.current();
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
  }, [router]);

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

  // ส่งออกข้อมูลทั้งหมดตามตัวกรองปัจจุบันเป็น CSV
  const handleExportCSV = async () => {
    try {
      setIsExporting(true);
      const params: any = { limit: 'all' };
      if (filterType !== 'all') params.name = filterType;
      if (filterGroup !== 'all') params.group_id = filterGroup;
      if (search.trim()) params.search = search.trim();

      const res = await eventService.getAll(params);
      const exportEvents = res.data.events || [];

      if (exportEvents.length === 0) {
        alert("ไม่มีข้อมูลสำหรับส่งออกตามตัวกรองที่เลือก");
        return;
      }

      let csv = "\uFEFFลำดับ,วันเวลา,สถานที่,ห้อง/จุดติดตั้ง,รหัสอุปกรณ์,ประเภทเหตุการณ์\n";
      exportEvents.forEach((e: any, idx: number) => {
        const eventText = getEventNameText(e.name);
        csv += `${idx + 1},"${formatDateTime(e.created_at)}","${e.group_name || '-'}","${e.device_name || '-'}","${e.device_code || '-'}","${eventText}"\n`;
      });

      const blob = new Blob([csv], { type: 'text/csv;charset=utf-8;' });
      const url = URL.createObjectURL(blob);
      const a = document.createElement('a');
      a.href = url;
      a.download = `fall_detection_history_${new Date().toISOString().slice(0, 10)}.csv`;
      a.click();
    } catch (err: any) {
      console.error("ส่งออก CSV ล้มเหลว:", err);
      alert("เกิดข้อผิดพลาดในการส่งออกไฟล์ CSV");
    } finally {
      setIsExporting(false);
    }
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
      <div className="grid grid-cols-1 sm:grid-cols-3 gap-3 sm:gap-5 mb-6">
        <div className="bg-white p-4 sm:p-5 rounded-2xl border border-gray-200/80 shadow-xs flex flex-col justify-between">
          <div className="flex items-center justify-between text-gray-500 text-xs mb-1">
            <span>การล้มทั้งหมด</span>
            <span className="p-1.5 bg-red-50 text-red-600 rounded-lg font-bold">🚨</span>
          </div>
          <div className="flex items-baseline gap-2">
            <span className="text-2xl sm:text-3xl font-extrabold text-gray-800">{stats.total_falls}</span>
            <span className="text-xs text-gray-400">ครั้ง</span>
          </div>
          <span className="text-[11px] text-gray-400 mt-2">
            {filterGroup === 'all' ? 'บันทึกทั้งหมดในระบบ' : 'บันทึกในสถานที่ที่เลือก'}
          </span>
        </div>

        <div className="bg-white p-4 sm:p-5 rounded-2xl border border-gray-200/80 shadow-xs flex flex-col justify-between">
          <div className="flex items-center justify-between text-gray-500 text-xs mb-1">
            <span>บันทึกเหตุการณ์ทั้งหมด</span>
            <span className="p-1.5 bg-blue-50 text-blue-600 rounded-lg font-bold">📋</span>
          </div>
          <div className="flex items-baseline gap-2">
            <span className="text-2xl sm:text-3xl font-extrabold text-blue-600">{total}</span>
            <span className="text-xs text-gray-400">รายการ</span>
          </div>
          <span className="text-[11px] text-gray-400 mt-2">
            {filterGroup === 'all' ? 'ตามเงื่อนไขตัวกรองปัจจุบัน' : 'ในสถานที่ที่เลือก'}
          </span>
        </div>

        <div className="bg-white p-4 sm:p-5 rounded-2xl border border-gray-200/80 shadow-xs flex flex-col justify-between">
          <div className="flex items-center justify-between text-gray-500 text-xs mb-1">
            <span>อุปกรณ์ที่เฝ้าระวัง</span>
            <span className="p-1.5 bg-emerald-50 text-emerald-600 rounded-lg font-bold">📡</span>
          </div>
          <div className="flex items-baseline gap-2">
            <span className="text-2xl sm:text-3xl font-extrabold text-emerald-600">
              {stats.online_devices} / {stats.total_devices}
            </span>
            <span className="text-xs text-emerald-600 font-medium">ออนไลน์</span>
          </div>
          <span className="text-[11px] text-gray-400 mt-2">
            {filterGroup === 'all' ? 'พร้อมตรวจจับตลอด 24 ชม.' : 'อุปกรณ์ในสถานที่นี้'}
          </span>
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
            <p className="text-xs text-gray-400">บันทึกวันเวลา สถานที่ และเหตุการณ์ความเคลื่อนไหวทั้งหมดที่ตรวจพบ</p>
          </div>

          <div className="flex flex-wrap items-center gap-2">
            <button 
              onClick={handleExportCSV} 
              disabled={isExporting || total === 0}
              className="px-3 py-1.5 text-xs font-medium text-gray-700 bg-gray-50 hover:bg-gray-100 border border-gray-200 rounded-xl transition flex items-center gap-1.5 disabled:opacity-50"
              title="ดาวน์โหลดข้อมูลทั้งหมดตามตัวกรองปัจจุบัน"
            >
              <Download size={14} className={isExporting ? "animate-bounce" : ""} />
              <span>{isExporting ? "กำลังส่งออก..." : `Export CSV (${total})`}</span>
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
        <div className="grid grid-cols-1 sm:grid-cols-3 gap-3 bg-gray-50 p-3.5 rounded-xl border border-gray-200/80">
          <div>
            <label className="block text-[11px] font-bold text-gray-600 mb-1 flex items-center gap-1">
              <Filter size={11} /> ประเภทเหตุการณ์
            </label>
            <select 
              value={filterType} 
              onChange={(e) => handleFilterTypeChange(e.target.value)} 
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
              onChange={(e) => handleFilterGroupChange(e.target.value)} 
              className="w-full text-xs bg-white border border-gray-300 rounded-lg p-2 focus:ring-2 focus:ring-blue-500 focus:outline-none"
            >
              <option value="all">ทุกสถานที่ (All Groups)</option>
              {groups.map((g) => (
                <option key={g.ID} value={g.ID}>{g.name}</option>
              ))}
            </select>
          </div>

          <div>
            <label className="block text-[11px] font-bold text-gray-600 mb-1 flex items-center gap-1">
              <Search size={11} /> ค้นหาห้อง / อุปกรณ์
            </label>
            <input 
              type="text" 
              value={search} 
              onChange={(e) => handleSearchChange(e.target.value)} 
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
                <th className="py-3 px-4">เหตุการณ์ที่เกิดขึ้น</th>
              </tr>
            </thead>
            <tbody className="divide-y divide-gray-100">
              {loading ? (
                <tr>
                  <td colSpan={3} className="py-8 text-center text-gray-400">
                    <RefreshCw className="animate-spin inline-block mr-2" size={18} />
                    กำลังโหลดข้อมูล...
                  </td>
                </tr>
              ) : events.length === 0 ? (
                <tr>
                  <td colSpan={3} className="py-8 text-center text-gray-400">
                    <div className="text-3xl mb-1">🔍</div>
                    <div>ไม่พบรายการเหตุการณ์ตามเงื่อนไขที่เลือก</div>
                  </td>
                </tr>
              ) : (
                events.map((ev) => {
                  const isFall = ev.name === 'fall';
                  const isProlongedStay = ev.name === 'prolonged_stay';

                  return (
                    <tr 
                      key={ev.id} 
                      className={`hover:bg-gray-50/80 transition-colors ${
                        isFall 
                          ? 'bg-red-50/40' 
                          : isProlongedStay 
                          ? 'bg-orange-50/40' 
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
                    </tr>
                  );
                })
              )}
            </tbody>
          </table>
        </div>

        {/* แถบควบคุมการแบ่งหน้า (Pagination Controls) & เลือกจำนวนต่อหน้า */}
        <div className="flex flex-col sm:flex-row items-center justify-between gap-4 pt-4 border-t border-gray-100 text-xs text-gray-500">
          {/* ซ้าย: แสดงจำนวนต่อหน้า และข้อความสรุป */}
          <div className="flex flex-wrap items-center gap-3 w-full sm:w-auto justify-between sm:justify-start">
            <div className="flex items-center gap-2">
              <span className="text-gray-600 font-medium">แสดงต่อหน้า:</span>
              <select
                value={pageSize}
                onChange={(e) => handlePageSizeChange(e.target.value)}
                className="bg-white border border-gray-300 rounded-lg py-1 px-2.5 text-xs text-gray-700 font-medium focus:ring-2 focus:ring-blue-500 focus:outline-none cursor-pointer"
              >
                <option value="50">50 รายการ</option>
                <option value="100">100 รายการ</option>
                <option value="250">250 รายการ</option>
                <option value="all">ทั้งหมด (All Events)</option>
              </select>
            </div>
            <span className="text-gray-400">
              {total === 0
                ? "ไม่พบข้อมูล"
                : pageSize === "all"
                ? `แสดงทั้งหมด ${total} รายการ`
                : `แสดง ${(page - 1) * Number(pageSize) + 1} - ${Math.min(page * Number(pageSize), total)} จาก ${total} รายการ`}
            </span>
          </div>

          {/* ขวา: ปุ่มเปลี่ยนหน้า (แสดงเฉพาะเมื่อไม่ได้เลือก "ทั้งหมด" และมีมากกว่า 1 หน้า) */}
          {pageSize !== "all" && totalPages > 1 && (
            <div className="flex items-center gap-1 w-full sm:w-auto justify-center sm:justify-end">
              <button
                onClick={() => setPage(1)}
                disabled={page === 1}
                className="p-1.5 rounded-lg border border-gray-200 text-gray-500 hover:bg-gray-100 disabled:opacity-40 disabled:hover:bg-transparent transition cursor-pointer"
                title="หน้าแรก"
              >
                <ChevronsLeft size={15} />
              </button>
              <button
                onClick={() => setPage((p) => Math.max(1, p - 1))}
                disabled={page === 1}
                className="p-1.5 rounded-lg border border-gray-200 text-gray-500 hover:bg-gray-100 disabled:opacity-40 disabled:hover:bg-transparent transition cursor-pointer"
                title="ก่อนหน้า"
              >
                <ChevronLeft size={15} />
              </button>

              <div className="flex items-center gap-1 mx-1">
                {getPageNumbers().map((pNum, idx) =>
                  pNum === "..." ? (
                    <span key={`dots-${idx}`} className="px-2 text-gray-400">
                      ...
                    </span>
                  ) : (
                    <button
                      key={`page-${pNum}`}
                      onClick={() => setPage(Number(pNum))}
                      className={`min-w-[28px] h-7 px-2 rounded-lg text-xs font-semibold transition cursor-pointer ${
                        page === pNum
                          ? "bg-blue-600 text-white shadow-xs"
                          : "border border-gray-200 text-gray-600 hover:bg-gray-100"
                      }`}
                    >
                      {pNum}
                    </button>
                  )
                )}
              </div>

              <button
                onClick={() => setPage((p) => Math.min(totalPages, p + 1))}
                disabled={page === totalPages}
                className="p-1.5 rounded-lg border border-gray-200 text-gray-500 hover:bg-gray-100 disabled:opacity-40 disabled:hover:bg-transparent transition cursor-pointer"
                title="ถัดไป"
              >
                <ChevronRight size={15} />
              </button>
              <button
                onClick={() => setPage(totalPages)}
                disabled={page === totalPages}
                className="p-1.5 rounded-lg border border-gray-200 text-gray-500 hover:bg-gray-100 disabled:opacity-40 disabled:hover:bg-transparent transition cursor-pointer"
                title="หน้าสุดท้าย"
              >
                <ChevronsRight size={15} />
              </button>
            </div>
          )}
        </div>
      </div>

    </div>
  );
}
