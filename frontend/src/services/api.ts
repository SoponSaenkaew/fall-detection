import axios from 'axios';

// ตั้งค่าให้ใช้พาร์ทสัมพัทธ์ /api/v1 เป็นค่าเริ่มต้นหากไม่ได้ประกาศใน Env (ช่วยให้ทำ Static Export ได้ราบรื่น)
const apiBaseURL = process.env.NEXT_PUBLIC_API_URL || '/api/v1';

export const api = axios.create({
  baseURL: apiBaseURL
});

export const authService = {
  login: (form: any) => api.post('/login', form),
  register: (form: any) => api.post('/register', form),
  getProfile: () => api.get('/iot/profile'),
};

export const groupService = {
  getAll: () => api.get('/iot/groups'),
  create: (name: string) => api.post('/iot/groups', { name }),
  update: (id: number, name: string) => api.put(`/iot/groups/${id}`, { name }),
  delete: (id: number) => api.delete(`/iot/groups/${id}`),
};

export const deviceService = {
  create: (data: any) => api.post('/iot/devices', data),
  update: (id: string, data: any) => api.put(`/iot/devices/${id}`, data),
  delete: (id: string) => api.delete(`/iot/devices/${id}`),
  getSettings: (deviceId: string) => api.get(`/iot/devices/${deviceId}/settings`),
  updateSettings: (data: any) => api.put(`/iot/devices/settings`, data),
};

export const notificationService = {
  save: (data: any) => api.post('/iot/notifications', data),
};

export default api;