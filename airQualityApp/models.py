from django.db import models
import datetime

# Create your models here.


class User(models.Model):

    def __str__(self):
        return f'{self.fullname}'

    fullname = models.fields.CharField(max_length=50)
    email = models.fields.CharField(max_length=50)
    phone_number = models.fields.CharField(max_length=50)
    username = models.fields.CharField(max_length=20)
    password = models.fields.CharField(max_length=10)
    created_at = models.fields.DateTimeField(default=datetime.datetime.now)


class Device(models.Model):

    class DeviceMode(models.IntegerChoices):
        MANUAL = 1
        AUTOMATIC = 2

    serial_number = models.fields.CharField(max_length=100)
    name = models.fields.CharField(max_length=20)
    mode = models.fields.IntegerField(choices=DeviceMode.choices, null=True)
    created_at = models.fields.DateTimeField(default=datetime.datetime.now)
    User = models.ForeignKey(User, null=True, on_delete=models.SET_NULL)


class Data(models.Model):
    quantityCO2 = models.fields.IntegerField(null=True)
    quantityCO = models.fields.IntegerField(null=True)
    quantityNO2 = models.fields.IntegerField(null=True)
    quantityO3 = models.fields.IntegerField(null=True)
    fine_particle = models.fields.IntegerField(null=True)
    temperature = models.fields.FloatField(null=True)
    created_at = models.fields.DateTimeField(default=datetime.datetime.now)
    Device = models.ForeignKey(Device, null=True, on_delete=models.SET_NULL)
