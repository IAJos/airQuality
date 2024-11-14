from django.shortcuts import render
from django.shortcuts import redirect
from airQualityApp.models import User, Device, Data
from airQualityApp.form import UserForm, DeviceForm
from airQualityApp.respone import Response_data
from django.http import HttpResponse
from django.http import JsonResponse


# Create your views here.


def no_found(request):
    return render(request, 'no_found.html')


def dashboard(request):
    return render(request, 'dashboard.html')


def data(request):
    users = User.objects.all()
    devices = Device.objects.all()
    data = Data.objects.all()
    return render(request, 'data.html', {
        'users': users,
        'devices': devices,
        'data': data
    })


def dashboard_view(request):
    users = User.objects.all()
    devices = Device.objects.all()
    data = Data.objects.all()
    return render(request, 'dashboard.html', {
        'users': users,
        'devices': devices,
        'data': data
    })


def get_devices_by_user(request, User_id):
    devices = Device.objects.filter(User_id=User_id)
    devices_data = [{'id': device.id, 'name': device.name} for device in devices]
    return JsonResponse(devices_data, safe=False)


def get_data_by_device(request, device_name_id):
    data = Data.objects.filter(device_name_id=device_name_id)
    data_info = [
        {
            'quantityCO2': d.quantityCO2,
            'quantityCO': d.quantityCO,
            'quantityNO2': d.quantityNO2,
            'quantityO3': d.quantityO3,
            'fine_particle': d.fine_particle,
            'temperature': d.temperature,
            'created_at': d.created_at
        } for d in data]
    return JsonResponse(data_info, safe=False)
