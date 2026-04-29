"use client";

import { useState, useEffect } from 'react'; // ✨ อาโรน่าเพิ่ม 2 ตัวนี้เข้ามานะคะ
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
import { Layout, Plus, Trash2, BellRing, Pencil, Settings2, X } from 'lucide-react';

export default function Dashboard() {
  // ดึง State และ Handler ทั้งหมดมาจาก Custom Hook ที่เซนเซย์จัดระเบียบไว้ค่ะ ✨
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

  // ✨ State สำหรับเก็บข้อความแจ้งเตือนฉุกเฉินค่ะ
  const [alertMsg, setAlertMsg] = useState("");

  // ✨ เปิดท่อ WebSocket ทันทีที่หน้าเว็บโหลดเสร็จ
  useEffect(() => {
    const ws = new WebSocket('ws://localhost:8080/ws');

    ws.onopen = () => console.log('เชื่อมต่อ WebSocket กับฐานทัพสำเร็จแล้วค่ะเซนเซย์! 🌐');

    ws.onmessage = (event) => {
      const data = JSON.parse(event.data);
      fetchData();
      // ถ้าข้อมูลที่ส่งมาคือการล้ม (fall) ให้แสดงแถบแจ้งเตือนทันที!
      if (data.name === 'fall') {
        setAlertMsg(`ตรวจพบผู้สูงอายุล้ม! ที่อุปกรณ์รหัส: ${data.device_id} รีบตรวจสอบด่วนค่ะ!`);
        // เซนเซย์สามารถเพิ่มเสียงแจ้งเตือนตรงนี้ได้ในอนาคตด้วยนะคะ 🔔
      }
    };

    return () => {
      ws.close();
      console.log('ปิดการเชื่อมต่อ WebSocket เรียบร้อยค่ะ 🔌');
    };
  }, []);

  // ฟังก์ชันช่วยจัดรูปแบบเวลาให้ดูง่ายค่ะ
  const formatTime = (dateString: string) => {
    const date = new Date(dateString);
    return date.toLocaleTimeString('th-TH', { hour: '2-digit', minute: '2-digit' });
  };

  return (
    <div className="p-8 bg-gray-50 min-h-screen">
      {/* 1. Header ส่วนหัวของ Dashboard */}
      <Header onAddGroup={() => setShowAddModal(true)} />

      {/* ✨ แถบแจ้งเตือนฉุกเฉิน (จะโผล่มาเฉพาะตอนที่มีคนล้มเท่านั้นค่ะ!) */}
      {alertMsg && (
        <div className="mt-6 mb-2 p-4 bg-red-500 text-white rounded-xl shadow-lg border-2 border-red-600 flex justify-between items-center animate-pulse">
          <div className="flex items-center">
            <BellRing className="mr-3" size={24} />
            <span className="font-bold text-lg">🚨 ฉุกเฉิน: {alertMsg}</span>
          </div>
          <button 
            onClick={() => setAlertMsg("")} 
            className="p-1 hover:bg-red-600 rounded-lg transition"
            title="ปิดการแจ้งเตือน"
          >
            <X size={24} />
          </button>
        </div>
      )}

      {/* 2. ส่วนของ Modals ทั้งหมด (ประกอบร่าง) */}
      
      {/* เพิ่มกลุ่ม/สถานที่ */}
      <AddGroupModal 
        isOpen={showAddModal} 
        onClose={() => setShowAddModal(false)} 
        onSubmit={handleAddGroup} 
        value={newGroupName} 
        onChange={setNewGroupName} 
      />

      {/* แก้ไขชื่อกลุ่ม/สถานที่ */}
      <EditGroupModal 
        isOpen={showEditGroup.show} 
        onClose={() => setShowEditGroup({show: false, id: null})} 
        onSubmit={handleUpdateGroup} 
        value={editGroupName} 
        onChange={setEditGroupName} 
      />

      {/* เพิ่มอุปกรณ์เซนเซอร์ */}
      <AddDeviceModal 
        show={showAddDevModal.show} 
        onClose={() => setShowAddDevModal({show: false, groupId: null})} 
        onSubmit={handleAddDevice} 
        newDev={newDev} 
        setNewDev={setNewDev} 
      />

      {/* แก้ไขข้อมูลอุปกรณ์ (Name & Hardware ID) */}
      <EditDeviceModal 
        show={showEditDev.show} 
        onClose={() => setShowEditDev({show: false, originalId: null})} 
        onSubmit={handleUpdateDevice} 
        devData={editDevData} 
        setDevData={setEditDevData} 
      />

      {/* ตั้งค่าพารามิเตอร์การล้ม/ความสูง/ขนาดห้อง */}
      <DeviceSettingsModal 
        show={showSettings.show} 
        onClose={() => setShowSettings({show: false, deviceId: ""})} 
        onSubmit={handleSaveSettings} 
        settings={currentSettings} 
        setSettings={setCurrentSettings} 
      />

      {/* ตั้งค่าการแจ้งเตือน Line/Webhook */}
      <NotificationModal 
        show={showNotifModal.show} 
        onClose={() => setShowNotifModal({show: false, groupId: null})} 
        onSubmit={handleSaveNotif} 
        form={notifForm} 
        setForm={setNotifForm} 
      />

      {/* 3. ส่วนแสดงผลกลุ่มและอุปกรณ์ (Main Content) */}
      <div className="grid grid-cols-1 md:grid-cols-2 lg:grid-cols-3 gap-6 mt-6">
        {groups.map((group) => (
          <div key={group.ID} className="bg-white p-6 rounded-2xl shadow-sm border border-gray-200 relative group">
            {/* ปุ่มจัดการกลุ่ม (จะปรากฏเมื่อ Hover) */}
            <div className="absolute top-4 right-4 flex gap-2 opacity-0 group-hover:opacity-100 transition">
              <button 
                onClick={() => {
                  setEditGroupName(group.name);
                  setShowEditGroup({show: true, id: group.ID});
                }} 
                className="text-gray-300 hover:text-blue-500 transition"
                title="แก้ไขชื่อสถานที่"
              >
                <Pencil size={18} />
              </button>
              <button 
                onClick={() => setShowNotifModal({show: true, groupId: group.ID})} 
                className="text-gray-300 hover:text-yellow-500 transition"
                title="ตั้งค่าแจ้งเตือน"
              >
                <BellRing size={18} />
              </button>
              <button 
                onClick={() => handleDeleteGroup(group.ID)} 
                className="text-gray-300 hover:text-red-500 transition"
                title="ลบกลุ่มนี้"
              >
                <Trash2 size={18} />
              </button>
            </div>
            
            <div className="flex items-center mb-4 text-blue-600 border-b pb-2 text-left">
              <Layout className="mr-2" />
              <h2 className="text-xl font-bold">{group.name}</h2>
            </div>
            
            <div className="space-y-4">
              {group.devices?.map((dev: any) => (
                <div key={dev.ID} className="relative group/dev">
                  {/* แผงควบคุมด่วนสำหรับอุปกรณ์แต่ละตัว */}
                  <div className="absolute top-2 right-8 z-10 flex gap-2 opacity-0 group-hover/dev:opacity-100 transition">
                    <button 
                      onClick={() => handleOpenSettings(dev.device_id)}
                      className="text-gray-300 hover:text-indigo-500"
                      title="ตั้งค่าพารามิเตอร์เซนเซอร์"
                    >
                      <Settings2 size={14} />
                    </button>
                    <button 
                      onClick={() => {
                        setEditDevData({ device_id: dev.device_id, name: dev.name });
                        setShowEditDev({ show: true, originalId: dev.device_id });
                      }}
                      className="text-gray-300 hover:text-blue-500"
                      title="แก้ไขข้อมูลอุปกรณ์"
                    >
                      <Pencil size={14} />
                    </button>
                  </div>

                  <DeviceCard 
                    dev={dev} 
                    formatTime={formatTime} 
                    onDelete={handleDeleteDevice} 
                  />
                </div>
              ))}
              
              <button 
                onClick={() => setShowAddDevModal({show: true, groupId: group.ID})} 
                className="w-full py-2 border-2 border-dashed border-blue-100 rounded-xl text-blue-400 text-xs hover:bg-blue-50 transition flex items-center justify-center font-medium"
              >
                <Plus size={14} className="mr-1" /> เพิ่มอุปกรณ์ใน {group.name}
              </button>
            </div>
          </div>
        ))}
      </div>
    </div>
  );
}