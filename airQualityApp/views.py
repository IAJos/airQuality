from django.shortcuts import render
from django.shortcuts import redirect
from airQualityApp.models import User, Device, Data
from airQualityApp.form import UserForm, DeviceForm
from airQualityApp.respone import Response_data


# Create your views here.


def hello(request):
    # bands = Band.objects.all()
    # return render(request, 'templates/hello.html', {'bands': bands})
    return render(request, 'dashboard.html')
    # return render(request, 'hello.html')


def get_data(request):
    users = User.objects.all()
    # devices = Device.objects.all().query.group_by('user_id')
    # data = Data.objects.all().query.group_by('device_id')

    responses = []

    for user in users:
        user_devices = Device.objects.filter(User_id=user.id)
        for device in user_devices:
            data_device = Data.objects.filter(Device_id=device.id)
            response = Response_data(
                user.fullname,
                device.name,
                500,
                5000,
                1250,
                566,
                device.id)
            responses.append(response)
            for data in data_device:
                response = Response_data(
                    user.fullname,
                    device.name,
                    data.quantityCO2,
                    data.quantityCO,
                    data.quantityO3,
                    data.fine_particle,
                    data.id)
                responses.append(response)

    return render(request, 'dashboard.html', {'responses': responses})


def user_create(request):
    if request.method == 'POST':
        form = UserForm(request.POST)
        if form.is_valid():
            user = form.save()
            return redirect('user-create')
    else:
        form = UserForm()
        return render(request, 'user_create.html', {'form': form})


def device_create(request):
    if request.method == 'POST':
        form = DeviceForm(request.POST)
        if form.is_valid():
            device = form.save()
            return redirect('device-create')
    else:
        form = DeviceForm()
        return render(request, 'device_create.html', {'form': form})


def about(request):
    return HttpResponse('<h1>À propos</h1> <p>Nous adorons IOTx !</p>')
