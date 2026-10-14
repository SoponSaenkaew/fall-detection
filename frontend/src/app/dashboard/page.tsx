"use client";

import { useState, useEffect } from 'react';
import { useRouter } from 'next/navigation';
import { useDashboard } from '@/hooks/useDashboard';
import DeviceCard from '@/components/Dashboard/DeviceCard';
import Header from '@/components/Dashboard/Header';
import { 
  AddGroupModal, 
  AddDeviceModal, 
  NotificationModal, 
  EditGroupModal, 
  EditDeviceModal,
  DeviceSettingsModal 
} from '@/components/Dashboard/Modals';
import { Layout, Plus, Trash2, BellRing, Pencil, X, Radio } from 'lucide-react';

export default function Dashboard() {
  const router = useRouter();
  // ดึงข้อมูลสถานะและตัวจัดการเหตุการณ์ทั้งหมดจาก Custom Hook
  const {
    groups, fetchData,
    showAddModal, setShowAddModal, 
    newGroupName, setNewGroupName,
    showAddDevModal, setShowAddDevModal, 
    newDev, setNewDev,
    showNotifModal, setShowNotifModal, 
    notifForm, setNotifForm,
    showEditGroup, setShowEditGroup, 
    editGroupName, setEditGroupName,
    showEditDev, setShowEditDev, 
    editDevData, setEditDevData,
    showSettings, setShowSettings, 
    currentSettings, setCurrentSettings,
    handleAddGroup, handleDeleteGroup, handleUpdateGroup,
    handleAddDevice, handleDeleteDevice, handleUpdateDevice,
    handleSaveNotif, handleOpenSettings, handleSaveSettings
  } = useDashboard();

  // State สำหรับเก็บข้อความแจ้งเตือนฉุกเฉิน
  const [alertMsg, setAlertMsg] = useState("");

  // ตรวจสอบการยืนยันตัวตนและเปิดท่อ WebSocket พร้อมระบบ Auto-Reconnect & Heartbeat ป้องกันท่อหลุด
  useEffect(() => {
    const token = typeof window !== 'undefined' ? localStorage.getItem('token') : null;
    if (!token) {
      router.push('/login');
      return;
    }

    let ws: WebSocket | null = null;
    let pingInterval: NodeJS.Timeout | null = null;
    let reconnectTimeout: NodeJS.Timeout | null = null;
    let isComponentMounted = true;

    const connectWebSocket = () => {
      if (!isComponentMounted) return;

      const wsProtocol = window.location.protocol === 'https:' ? 'wss:' : 'ws:';
      const wsHost = window.location.host;
      const wsUrl = (window.location.hostname === 'localhost' || window.location.hostname === '127.0.0.1')
        ? 'ws://localhost:8080/ws'
        : `${wsProtocol}//${wsHost}/ws`;

      ws = new WebSocket(wsUrl);

      ws.onopen = () => {
        console.log('✅ เชื่อมต่อ WebSocket สำเร็จ (Live Realtime)');
        fetchData();

        // ส่ง Heartbeat Ping ทุกๆ 25 วินาที เพื่อเลี้ยงไม่ให้ Nginx ตัด Timeout 60s
        if (pingInterval) clearInterval(pingInterval);
        pingInterval = setInterval(() => {
          if (ws && ws.readyState === WebSocket.OPEN) {
            ws.send(JSON.stringify({ type: 'ping' }));
          }
        }, 25000);
      };

      ws.onmessage = (event) => {
        try {
          const data = JSON.parse(event.data);
          fetchData();
          if (data.name === 'fall') {
            setAlertMsg(`ตรวจพบเหตุการณ์การล้มจากอุปกรณ์รหัส: ${data.device_id} โปรดตรวจสอบโดยด่วน`);
          }
        } catch (e) {
          // ignore non-json messages
        }
      };

      ws.onclose = (e) => {
        if (pingInterval) clearInterval(pingInterval);
        if (isComponentMounted) {
          console.warn('⚠️ WebSocket ขาดการติดต่อ กำลังเชื่อมต่อใหม่ใน 3 วินาที...', e.reason);
          if (reconnectTimeout) clearTimeout(reconnectTimeout);
          reconnectTimeout = setTimeout(connectWebSocket, 3000);
        }
      };

      ws.onerror = (err) => {
        console.error('❌ WebSocket Error:', err);
        ws?.close();
      };
    };

    connectWebSocket();

    // ดึงข้อมูลใหม่และต่อ WebSocket ทันทีเมื่อผู้ใช้สลับกลับมาที่หน้าเว็บนี้ (Tab Focus/Visibility)
    const handleVisibilityChange = () => {
      if (document.visibilityState === 'visible') {
        fetchData();
        if (!ws || ws.readyState === WebSocket.CLOSED || ws.readyState === WebSocket.CLOSING) {
          connectWebSocket();
        }
      }
    };
    document.addEventListener('visibilitychange', handleVisibilityChange);

    // ระบบสำรอง Polling อัตโนมัติทุกๆ 15 วินาที เพื่อความมั่นใจว่าข้อมูลไม่ค้าง
    const fallbackPoll = setInterval(fetchData, 15000);

    return () => {
      isComponentMounted = false;
      document.removeEventListener('visibilitychange', handleVisibilityChange);
      if (pingInterval) clearInterval(pingInterval);
      if (reconnectTimeout) clearTimeout(reconnectTimeout);
      clearInterval(fallbackPoll);
      if (ws) ws.close();
      console.log('ปิดการเชื่อมต่อ WebSocket เรียบร้อยแล้ว');
    };
  }, [router]);

  // ฟังก์ชันช่วยจัดรูปแบบเวลาให้ดูง่าย
  const formatTime = (dateString: string) => {
    if (!dateString) return '-';
    const date = new Date(dateString);
    return date.toLocaleTimeString('th-TH', { hour: '2-digit', minute: '2-digit' });
  };

  // คำนวณสถิติภาพรวมสำหรับดูสถานะคร่าวๆ
  const totalDevices = groups.reduce((acc, g) => acc + (g.devices?.length || 0), 0);
  const onlineDevices = groups.reduce((acc, g) => acc + (g.devices?.filter((d: any) => d.status === 'online').length || 0), 0);
  const offlineDevices = totalDevices - onlineDevices;
  const allExistingDeviceIds = groups.flatMap((g) => (g.devices || []).map((d: any) => d.device_id));

  return (
    <div className="p-4 sm:p-8 bg-slate-50 min-h-screen">
      {/* 1. Header ส่วนหัวของ Dashboard */}
      <Header onAddGroup={() => setShowAddModal(true)} />

      {/* แถบแจ้งเตือนสั้นๆ หากมีเหตุการณ์ */}
      {alertMsg && (
        <div className="mb-6 p-4 bg-amber-50 border border-amber-200 text-amber-900 rounded-2xl shadow-xs flex justify-between items-center">
          <div className="flex items-center gap-2.5">
            <span className="p-1.5 bg-amber-100 text-amber-700 rounded-xl font-bold">⚠️</span>
            <span className="font-semibold text-xs sm:text-sm">{alertMsg}</span>
          </div>
          <button 
            onClick={() => setAlertMsg("")} 
            className="p-1 hover:bg-amber-100 rounded-lg transition text-amber-700"
            title="ปิดการแจ้งเตือน"
          >
            <X size={18} />
          </button>
        </div>
      )}

      {/* 2. สรุปภาพรวมสำหรับดูสถานะคร่าวๆ (Quick Glance Overview) */}
      <div className="grid grid-cols-2 lg:grid-cols-4 gap-3 sm:gap-4 mb-6">
        <div className="bg-white p-4 rounded-2xl border border-slate-200/80 shadow-2xs flex items-center gap-3.5 text-left">
          <div className="w-11 h-11 rounded-xl bg-blue-50 text-blue-600 flex items-center justify-center flex-shrink-0">
            <Layout size={20} />
          </div>
          <div>
            <p className="text-[11px] font-semibold text-slate-400">สถานที่ / อาคาร</p>
            <p className="text-xl font-bold text-slate-800">{groups.length} <span className="text-xs font-normal text-slate-400">แห่ง</span></p>
          </div>
        </div>

        <div className="bg-white p-4 rounded-2xl border border-slate-200/80 shadow-2xs flex items-center gap-3.5 text-left">
          <div className="w-11 h-11 rounded-xl bg-indigo-50 text-indigo-600 flex items-center justify-center flex-shrink-0">
            <Radio size={20} />
          </div>
          <div>
            <p className="text-[11px] font-semibold text-slate-400">เซนเซอร์ทั้งหมด</p>
            <p className="text-xl font-bold text-slate-800">{totalDevices} <span className="text-xs font-normal text-slate-400">ตัว</span></p>
          </div>
        </div>

        <div className="bg-white p-4 rounded-2xl border border-slate-200/80 shadow-2xs flex items-center gap-3.5 text-left">
          <div className="w-11 h-11 rounded-xl bg-emerald-50 text-emerald-600 flex items-center justify-center flex-shrink-0">
            <span className="w-3 h-3 rounded-full bg-emerald-500 animate-ping"></span>
          </div>
          <div>
            <p className="text-[11px] font-semibold text-slate-400">ออนไลน์ (Online)</p>
            <p className="text-xl font-bold text-emerald-600">{onlineDevices} <span className="text-xs font-normal text-slate-400">ตัว</span></p>
          </div>
        </div>

        <div className="bg-white p-4 rounded-2xl border border-slate-200/80 shadow-2xs flex items-center gap-3.5 text-left">
          <div className="w-11 h-11 rounded-xl bg-slate-100 text-slate-400 flex items-center justify-center flex-shrink-0">
            <span className="w-3 h-3 rounded-full bg-slate-400"></span>
          </div>
          <div>
            <p className="text-[11px] font-semibold text-slate-400">ออฟไลน์ (Offline)</p>
            <p className="text-xl font-bold text-slate-500">{offlineDevices} <span className="text-xs font-normal text-slate-400">ตัว</span></p>
          </div>
        </div>
      </div>

      {/* 3. ส่วนของ Modals ทั้งหมด */}
      <AddGroupModal 
        isOpen={showAddModal} 
        onClose={() => setShowAddModal(false)} 
        onSubmit={handleAddGroup} 
        value={newGroupName} 
        onChange={setNewGroupName} 
      />

      <EditGroupModal 
        isOpen={showEditGroup.show} 
        onClose={() => setShowEditGroup({show: false, id: null})} 
        onSubmit={handleUpdateGroup} 
        value={editGroupName} 
        onChange={setEditGroupName} 
      />

      <AddDeviceModal 
        show={showAddDevModal.show} 
        onClose={() => setShowAddDevModal({show: false, groupId: null})} 
        onSubmit={handleAddDevice} 
        newDev={newDev} 
        setNewDev={setNewDev} 
        existingDeviceIds={allExistingDeviceIds}
      />

      <EditDeviceModal 
        show={showEditDev.show} 
        onClose={() => setShowEditDev({show: false, originalId: null})} 
        onSubmit={handleUpdateDevice} 
        devData={editDevData} 
        setDevData={setEditDevData} 
        originalId={showEditDev.originalId}
        existingDeviceIds={allExistingDeviceIds}
      />

      <DeviceSettingsModal 
        show={showSettings.show} 
        onClose={() => setShowSettings({show: false, deviceId: ""})} 
        onSubmit={handleSaveSettings} 
        settings={currentSettings} 
        setSettings={setCurrentSettings} 
      />

      <NotificationModal 
        show={showNotifModal.show} 
        onClose={() => setShowNotifModal({show: false, groupId: null})} 
        onSubmit={handleSaveNotif} 
        form={notifForm} 
        setForm={setNotifForm} 
      />

      {/* 4. ส่วนแสดงผลกลุ่มและอุปกรณ์ แยกตามสถานที่ (Group Sections) */}
      <div className="space-y-6 text-left">
        {groups.map((group) => {
          const groupDevices = group.devices || [];
          const groupOnlineCount = groupDevices.filter((d: any) => d.status === 'online').length;

          return (
            <div key={group.ID} className="bg-white rounded-3xl border border-slate-200/90 shadow-2xs p-5 sm:p-6 transition">
              {/* แถบหัวข้อสถานที่และปุ่มควบคุม */}
              <div className="flex flex-col sm:flex-row sm:items-center justify-between gap-3 pb-4 border-b border-slate-100">
                <div className="flex items-center gap-3">
                  <div className="w-10 h-10 rounded-xl bg-blue-600 text-white flex items-center justify-center font-bold shadow-xs">
                    <Layout size={20} />
                  </div>
                  <div>
                    <div className="flex items-center gap-2">
                      <h2 className="text-base sm:text-lg font-bold text-slate-800">{group.name}</h2>
                      <span className="text-[11px] font-semibold bg-slate-100 text-slate-600 px-2 py-0.5 rounded-lg">
                        {groupDevices.length} อุปกรณ์ (ออนไลน์ {groupOnlineCount})
                      </span>
                    </div>
                    <p className="text-xs text-slate-400">จัดการเซนเซอร์เรดาร์และการแจ้งเตือนประจำอาคารนี้</p>
                  </div>
                </div>

                {/* ปุ่มจัดการประจำสถานที่ (LINE Alert, Edit, Delete, Add Device) */}
                <div className="flex flex-wrap items-center gap-2">
                  <button 
                    type="button"
                    onClick={() => setShowNotifModal({show: true, groupId: group.ID})} 
                    className="px-3 py-1.5 text-xs font-semibold text-amber-700 bg-amber-50 hover:bg-amber-100 border border-amber-200/80 rounded-xl transition flex items-center gap-1.5"
                    title="ตั้งค่าแจ้งเตือนผ่าน LINE"
                  >
                    <BellRing size={14} />
                    <span>แจ้งเตือน LINE</span>
                  </button>

                  <button 
                    type="button"
                    onClick={() => {
                      setEditGroupName(group.name);
                      setShowEditGroup({show: true, id: group.ID});
                    }} 
                    className="p-1.5 text-slate-500 hover:text-blue-600 hover:bg-blue-50 border border-slate-200 rounded-xl transition"
                    title="แก้ไขชื่อสถานที่"
                  >
                    <Pencil size={14} />
                  </button>

                  <button 
                    type="button"
                    onClick={() => handleDeleteGroup(group.ID)} 
                    className="p-1.5 text-slate-400 hover:text-red-600 hover:bg-red-50 border border-slate-200 rounded-xl transition"
                    title="ลบสถานที่นี้"
                  >
                    <Trash2 size={14} />
                  </button>

                  <button 
                    type="button"
                    onClick={() => setShowAddDevModal({show: true, groupId: group.ID})} 
                    className="px-3 py-1.5 text-xs font-bold text-white bg-blue-600 hover:bg-blue-700 rounded-xl transition shadow-xs flex items-center gap-1 ml-1"
                  >
                    <Plus size={14} />
                    <span>เพิ่มเซนเซอร์</span>
                  </button>
                </div>
              </div>

              {/* รายการการ์ดอุปกรณ์ในสถานที่นี้ */}
              <div className="mt-4">
                {groupDevices.length > 0 ? (
                  <div className="grid grid-cols-1 md:grid-cols-2 lg:grid-cols-3 gap-4">
                    {groupDevices.map((dev: any) => (
                      <DeviceCard 
                        key={dev.ID}
                        dev={dev} 
                        formatTime={formatTime} 
                        onDelete={handleDeleteDevice}
                        onOpenSettings={handleOpenSettings}
                        onEdit={(d: any) => {
                          setEditDevData({ device_id: d.device_id, name: d.name });
                          setShowEditDev({ show: true, originalId: d.device_id });
                        }}
                      />
                    ))}
                  </div>
                ) : (
                  <div className="py-10 text-center text-slate-400 bg-slate-50 rounded-2xl border border-dashed border-slate-200 flex flex-col items-center justify-center">
                    <Radio size={28} className="text-slate-300 mb-2" />
                    <p className="text-xs font-medium">ยังไม่มีเซนเซอร์เรดาร์ในสถานที่นี้</p>
                    <button 
                      type="button"
                      onClick={() => setShowAddDevModal({show: true, groupId: group.ID})} 
                      className="mt-3 px-3.5 py-1.5 text-xs font-bold text-blue-600 bg-white hover:bg-blue-50 border border-blue-200 rounded-xl transition"
                    >
                      + ลงทะเบียนเซนเซอร์ตัวแรก
                    </button>
                  </div>
                )}
              </div>
            </div>
          );
        })}

        {groups.length === 0 && (
          <div className="py-16 text-center text-slate-400 bg-white rounded-3xl border border-dashed border-slate-300 flex flex-col items-center justify-center">
            <Layout size={36} className="text-slate-300 mb-3" />
            <h3 className="text-base font-bold text-slate-700">ยังไม่มีสถานที่หรืออาคารในระบบ</h3>
            <p className="text-xs text-slate-400 mt-1 max-w-sm">
              เริ่มต้นด้วยการเพิ่มสถานที่ เช่น &quot;บ้านพัก&quot;, &quot;อาคาร 1&quot; เพื่อจัดกลุ่มเซนเซอร์เรดาร์
            </p>
            <button 
              type="button"
              onClick={() => setShowAddModal(true)} 
              className="mt-4 px-5 py-2.5 bg-blue-600 hover:bg-blue-700 text-white text-xs font-bold rounded-xl transition shadow-md flex items-center gap-1.5"
            >
              <Plus size={16} />
              <span>สร้างสถานที่แรก</span>
            </button>
          </div>
        )}
      </div>
    </div>
  );
}
