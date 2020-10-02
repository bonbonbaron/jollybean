def is_valid(obj):
    for key in obj.__dict__.keys():
        if obj.__dict__[key] is None:
            return False
    return True
        
