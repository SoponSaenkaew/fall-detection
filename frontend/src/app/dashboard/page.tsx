"use client";
import { useDashboard } from '@/hooks/useDashboard';
import DeviceCard from '@/components/Dashboard/DeviceCard';
import Header from '@/components/Dashboard/Header';
// ✨ เพิ่ม EditGroupModal และ EditDeviceModal เข้ามาในรายการ import ค่ะ
import { 
  AddGroupModal, 
  AddDeviceModal, 
  NotificationModal, 
  EditGroupModal, 
  EditDeviceModal 
} from '@/components/Dashboard/Modals';
import { Layout, Plus, Trash2, BellRing, Pencil } from 'lucide-react';

export default function Dashboard() {
  const {
    groups, showAddModal, setShowAddModal, newGroupName, setNewGroupName,
    showAddDevModal, setShowAddDevModal, newDev, setNewDev,
    showNotifModal, setShowNotifModal, notifForm, setNotifForm,
    handleAddGroup, handleDeleteGroup, handleAddDevice, handleDeleteDevice, handleSaveNotif,
    // ✨ ดึง State และ Handler ใหม่สำหรับการแก้ไขออกมาจาก Hook ค่ะ
    showEditGroup, setShowEditGroup, editGroupName, setEditGroupName,
    showEditDev, setShowEditDev, editDevData, setEditDevData,
    handleUpdateGroup, handleUpdateDevice
  } = useDashboard();

  const formatTime = (dateString: string) => {
    const date = new Date(dateString);
    return date.toLocaleTimeString('th-TH', { hour: '2-digit', minute: '2-digit' });
  };

  return (
    <div className="p-8 bg-gray-50 min-h-screen">
      <Header onAddGroup={() => setShowAddModal(true)} />

      {/* --- รวม Modals ไว้ที่จุดเดียว --- */}
      <AddGroupModal isOpen={showAddModal} onClose={() => setShowAddModal(false)} onSubmit={handleAddGroup} value={newGroupName} onChange={setNewGroupName} />
      <NotificationModal show={showNotifModal.show} onClose={() => setShowNotifModal({show: false, groupId: null})} onSubmit={handleSaveNotif} form={notifForm} setForm={setNotifForm} />
      <AddDeviceModal show={showAddDevModal.show} onClose={() => setShowAddDevModal({show: false, groupId: null})} onSubmit={handleAddDevice} newDev={newDev} setNewDev={setNewDev} />

      {/* ✨ เพิ่ม Modals สำหรับการแก้ไขเข้าไปค่ะ */}
      <EditGroupModal 
        isOpen={showEditGroup.show} 
        onClose={() => setShowEditGroup({show: false, id: null})} 
        onSubmit={handleUpdateGroup} 
        value={editGroupName} 
        onChange={setEditGroupName} 
      />
      <EditDeviceModal 
        show={showEditDev.show} 
        onClose={() => setShowEditDev({show: false, originalId: null})} 
        onSubmit={handleUpdateDevice} 
        devData={editDevData} 
        setDevData={setEditDevData} 
      />

      {/* --- ตารางแสดงผลกลุ่ม --- */}
      <div className="grid grid-cols-1 md:grid-cols-2 lg:grid-cols-3 gap-6">
        {groups.map((group) => (
          <div key={group.ID} className="bg-white p-6 rounded-2xl shadow-sm border border-gray-200 relative group">
            <div className="absolute top-4 right-4 flex gap-2 opacity-0 group-hover:opacity-100 transition">
              {/* ✨ ปุ่มแก้ไขชื่อกลุ่ม (ดินสอ) */}
              <button 
                onClick={() => {
                  setEditGroupName(group.name);
                  setShowEditGroup({show: true, id: group.ID});
                }} 
                className="text-gray-400 hover:text-blue-500" 
                title="แก้ไขชื่อสถานที่"
              >
                <Pencil size={18} />
              </button>
              
              <button onClick={() => setShowNotifModal({show: true, groupId: group.ID})} className="text-gray-400 hover:text-yellow-500" title="ตั้งค่าการแจ้งเตือน"><BellRing size={18} /></button>
              <button onClick={() => handleDeleteGroup(group.ID)} className="text-gray-400 hover:text-red-500" title="ลบกลุ่มนี้"><Trash2 size={18} /></button>
            </div>
            
            <h2 className="text-xl font-bold flex items-center mb-4 text-blue-600 border-b pb-2 text-left"><Layout className="mr-2" />{group.name}</h2>
            
            <div className="space-y-4">
              {group.devices?.map((dev: any) => (
                <div key={dev.ID} className="relative group/dev">
                  {/* ✨ ปุ่มแก้ไขอุปกรณ์ (แสดงเมื่อเอาเมาส์ชี้ที่ตัวอุปกรณ์) */}
                  <button 
                    onClick={() => {
                      setEditDevData({ device_id: dev.device_id, name: dev.name });
                      setShowEditDev({ show: true, originalId: dev.device_id });
                    }}
                    className="absolute top-2 right-8 z-10 text-gray-300 hover:text-blue-500 transition opacity-0 group-hover/dev:opacity-100"
                    title="แก้ไขข้อมูลอุปกรณ์"
                  >
                    <Pencil size={14} />
                  </button>

                  <DeviceCard 
                    dev={dev} 
                    formatTime={formatTime} 
                    onDelete={handleDeleteDevice} 
                  />
                </div>
              ))}
              <button onClick={() => setShowAddDevModal({show: true, groupId: group.ID})} className="w-full py-2 border-2 border-dashed border-blue-100 rounded-xl text-blue-400 text-xs hover:bg-blue-50 transition flex items-center justify-center font-medium">
                <Plus size={14} className="mr-1" /> เพิ่มอุปกรณ์
              </button>
            </div>
          </div>
        ))}
      </div>
    </div>
  );
}