import axios from 'axios';

const token = typeof window !== 'undefined' ? localStorage.getItem('token') : null;

const api = axios.create({
  baseURL: process.env.NEXT_PUBLIC_API_URL,
  headers: { Authorization: `Bearer ${token}` }
});

export const groupService = {
  getAll: () => api.get('/iot/groups'),
  create: (name: string) => api.post('/iot/groups', { name }),
  delete: (id: number) => api.delete(`/iot/groups/${id}`),
};

export const deviceService = {
  create: (data: any) => api.post('/iot/devices', data),
  delete: (id: string) => api.delete(`/iot/devices/${id}`),
};

export const notificationService = {
  save: (data: any) => api.post('/iot/notifications', data),
};