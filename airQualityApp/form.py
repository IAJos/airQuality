from django import forms
from airQualityApp.models import User, Device


class UserForm(forms.ModelForm):
    class Meta:
        model = User
        # fields = '__all__'
        exclude = ('created_at',)


class DeviceForm(forms.ModelForm):
    class Meta:
        model = Device
        # fields = '__all__'
        exclude = ('created_at',)
