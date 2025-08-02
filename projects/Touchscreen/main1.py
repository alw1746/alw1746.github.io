import os
from kivy.app import App
from kivy.uix.screenmanager import ScreenManager, Screen, FadeTransition
from kivy.uix.boxlayout import BoxLayout
from kivy.uix.image import AsyncImage
from kivy.uix.label import Label
from kivy.uix.button import Button
from kivy.core.window import Window
from kivy.graphics import Color, Rectangle

# Path to media directory
MEDIA_DIR = os.path.join(os.path.dirname(__file__), 'media')

# Helper to load sorted file list
def load_images(category):
    path = os.path.join(MEDIA_DIR, category)
    files = sorted([f for f in os.listdir(path)
                    if f.lower().endswith(('.png', '.jpg', '.jpeg', '.gif'))])
    return [os.path.join(path, f) for f in files]

class MainMenu(Screen):
    def __init__(self, **kwargs):
        super().__init__(**kwargs)
        layout = BoxLayout(orientation='vertical', spacing=10, padding=20)
        
        title = Label(text='Main Menu', size_hint_y=None, height=50, font_size='24sp')
        layout.add_widget(title)
        
        btn_layout = BoxLayout()
        for category in ['Cars', 'Motorcycles', 'Planes', 'Ships']:
            btn = Button(text=category, on_release=self.open_category)
            btn_layout.add_widget(btn)
        layout.add_widget(btn_layout)
        
        self.add_widget(layout)
    
    def open_category(self, instance):
        category = instance.text
        self.manager.current = category
        self.manager.get_screen(category).load_category()

class CategoryScreen(Screen):
    def __init__(self, **kwargs):
        super().__init__(**kwargs)
        self.category = None
        self.images = []
        self.index = 0

        self.root_layout = BoxLayout(orientation='vertical')
        # Display pane (80%)
        self.display_pane = BoxLayout(size_hint_y=0.8)
        self.img = AsyncImage()
        self.display_pane.add_widget(self.img)
        self.root_layout.add_widget(self.display_pane)
        
        # Instruction pane (20%)
        instr = BoxLayout(size_hint_y=0.2, padding=10)
        instr_left = BoxLayout()
        instr_left.add_widget(Button(background_normal='assets/icons/arrow_left.png',
                                     size_hint_x=None, width=50))
        instr_left.add_widget(Label(text='Next'))
        
        instr_center = BoxLayout()
        instr_center.add_widget(Button(background_normal='assets/icons/arrow_up.png',
                                       size_hint_x=None, width=50))
        instr_center.add_widget(Label(text='Main Menu'))
        
        instr_right = BoxLayout()
        instr_right.add_widget(Button(background_normal='assets/icons/arrow_right.png',
                                      size_hint_x=None, width=50))
        instr_right.add_widget(Label(text='Previous'))
        
        instr.add_widget(instr_left)
        instr.add_widget(instr_center)
        instr.add_widget(instr_right)
        self.root_layout.add_widget(instr)
        
        # Bind gestures
        Window.bind(on_touch_up=self.on_touch_up)
        
        self.add_widget(self.root_layout)
    
    def load_category(self):
        self.category = self.manager.current
        self.images = load_images(self.category)
        self.index = 0
        if self.images:
            self.img.source = self.images[0]
    
    def on_touch_up(self, window, touch):
        x, y = touch.pos
        w, h = Window.size
        dx = touch.dx
        dy = touch.dy
        
        # Swipe thresholds
        thr_x = w * 0.1
        thr_y = h * 0.1
        
        if dy > thr_y:
            # Swipe up: back to menu
            self.manager.current = 'main'
        elif dx < -thr_x:
            # Swipe left: next image
            self.next_image()
        elif dx > thr_x:
            # Swipe right: previous image
            self.prev_image()
        return True
    
    def next_image(self):
        if not self.images:
            return
        self.index = (self.index + 1) % len(self.images)
        self.img.source = self.images[self.index]
    
    def prev_image(self):
        if not self.images:
            return
        self.index = (self.index - 1) % len(self.images)
        self.img.source = self.images[self.index]

class TransportApp(App):
    def build(self):
        self.title = 'Transport Vehicles'
        sm = ScreenManager(transition=FadeTransition())
        sm.add_widget(MainMenu(name='main'))
        
        for category in ['Cars', 'Motorcycles', 'Planes', 'Ships']:
            screen = CategoryScreen(name=category)
            sm.add_widget(screen)
        
        return sm

if __name__ == '__main__':
    TransportApp().run()