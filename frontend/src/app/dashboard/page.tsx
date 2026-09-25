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
import { Layout, Plus, Trash2, BellRing, Pencil, X } from 'lucide-react';

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

  return (
    <div className="p-4 sm:p-8 bg-gray-50 min-h-screen">
      {/* 1. Header ส่วนหัวของ Dashboard */}
      <Header onAddGroup={() => setShowAddModal(true)} />

      {/* แถบแจ้งเตือนฉุกเฉิน */}
      {alertMsg && (
        <div className="mt-4 sm:mt-6 mb-4 p-4 bg-red-500 text-white rounded-2xl shadow-lg border-2 border-red-600 flex justify-between items-center animate-pulse">
          <div className="flex items-center">
            <BellRing className="mr-3 flex-shrink-0" size={24} />
            <span className="font-bold text-base sm:text-lg">🚨 ฉุกเฉิน: {alertMsg}</span>
          </div>
          <button 
            onClick={() => setAlertMsg("")} 
            className="p-1.5 hover:bg-red-600 active:bg-red-700 rounded-xl transition flex-shrink-0"
            title="ปิดการแจ้งเตือน"
          >
            <X size={20} />
          </button>
        </div>
      )}

      {/* 2. ส่วนของ Modals ทั้งหมด */}
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
      />

      <EditDeviceModal 
        show={showEditDev.show} 
        onClose={() => setShowEditDev({show: false, originalId: null})} 
        onSubmit={handleUpdateDevice} 
        devData={editDevData} 
        setDevData={setEditDevData} 
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

      {/* 3. ส่วนแสดงผลกลุ่มและอุปกรณ์ (Main Content) */}
      <div className="grid grid-cols-1 md:grid-cols-2 lg:grid-cols-3 gap-5 sm:gap-6 mt-4 sm:mt-6">
        {groups.map((group) => (
          <div key={group.ID} className="bg-white p-5 sm:p-6 rounded-2xl shadow-xs border border-gray-200 flex flex-col justify-between">
            <div>
              {/* แถบหัวข้อกลุ่มพร้อมปุ่มจัดการที่มองเห็นและกดง่ายบนมือถือ */}
              <div className="flex items-center justify-between mb-4 border-b pb-3 text-left gap-2">
                <div className="flex items-center text-blue-600 min-w-0">
                  <div className="w-8 h-8 rounded-lg bg-blue-50 text-blue-600 flex items-center justify-center mr-2.5 flex-shrink-0">
                    <Layout size={18} />
                  </div>
                  <h2 className="text-base sm:text-lg font-bold text-gray-800 truncate" title={group.name}>{group.name}</h2>
                </div>

                {/* ปุ่มจัดการกลุ่ม (มองเห็นชัดเจนทั้งบนมือถือและจอคอม) */}
                <div className="flex items-center gap-1 flex-shrink-0 bg-gray-50/80 p-1 rounded-xl border border-gray-200/70">
                  <button 
                    onClick={() => {
                      setEditGroupName(group.name);
                      setShowEditGroup({show: true, id: group.ID});
                    }} 
                    className="p-1.5 text-gray-500 hover:text-blue-600 hover:bg-blue-50 active:bg-blue-100 rounded-lg transition"
                    title="แก้ไขชื่อสถานที่"
                    aria-label="แก้ไขสถานที่"
                  >
                    <Pencil size={15} />
                  </button>
                  <button 
                    onClick={() => setShowNotifModal({show: true, groupId: group.ID})} 
                    className="p-1.5 text-gray-500 hover:text-amber-600 hover:bg-amber-50 active:bg-amber-100 rounded-lg transition"
                    title="ตั้งค่าแจ้งเตือน LINE/Webhook"
                    aria-label="ตั้งค่าแจ้งเตือน"
                  >
                    <BellRing size={15} />
                  </button>
                  <button 
                    onClick={() => handleDeleteGroup(group.ID)} 
                    className="p-1.5 text-gray-400 hover:text-red-600 hover:bg-red-50 active:bg-red-100 rounded-lg transition"
                    title="ลบกลุ่มนี้"
                    aria-label="ลบกลุ่ม"
                  >
                    <Trash2 size={15} />
                  </button>
                </div>
              </div>
              
              {/* รายการอุปกรณ์ภายใต้กลุ่ม */}
              <div className="space-y-3 mb-4">
                {group.devices && group.devices.length > 0 ? (
                  group.devices.map((dev: any) => (
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
                  ))
                ) : (
                  <div className="py-8 text-center text-gray-400 text-xs bg-slate-50 rounded-xl border border-dashed border-gray-200">
                    ยังไม่มีอุปกรณ์ในกลุ่มนี้
                  </div>
                )}
              </div>
            </div>

            {/* ปุ่มเพิ่มอุปกรณ์ใหม่ในกลุ่ม */}
            <button 
              onClick={() => setShowAddDevModal({show: true, groupId: group.ID})} 
              className="w-full py-2.5 px-3 border-2 border-dashed border-blue-200/80 rounded-xl text-blue-600 text-xs font-semibold hover:bg-blue-50 active:bg-blue-100 transition flex items-center justify-center gap-1.5 mt-2"
            >
              <Plus size={16} /> 
              <span>เพิ่มอุปกรณ์ใน {group.name}</span>
            </button>
          </div>
        ))}
      </div>
    </div>
  );
}
