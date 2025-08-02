import os
from kivy.app import App
from kivy.uix.screenmanager import ScreenManager, Screen
from kivy.uix.boxlayout import BoxLayout
from kivy.uix.gridlayout import GridLayout
from kivy.uix.label import Label
from kivy.uix.button import Button
from kivy.uix.image import Image
from kivy.core.window import Window


def load_category_images(media_dir='media'):
    """Scan each subfolder in media_dir for image files and return a dict."""
    exts = ('.png', '.jpg', '.jpeg', '.gif', '.bmp')
    categories = {}
    for cat in sorted(os.listdir(media_dir)):
        cat_path = os.path.join(media_dir, cat)
        if os.path.isdir(cat_path):
            files = [f for f in os.listdir(cat_path)
                     if f.lower().endswith(exts) and
                     os.path.isfile(os.path.join(cat_path, f))]
            files.sort()
            fullpaths = [os.path.join(cat_path, f) for f in files]
            if fullpaths:
                categories[cat] = fullpaths
    return categories


class MainMenuScreen(Screen):
    def __init__(self, categories, **kwargs):
        super().__init__(**kwargs)
        layout = BoxLayout(orientation='vertical', padding=20, spacing=20)
        title = Label(text='Main Menu', size_hint_y=0.2, font_size='24sp')
        grid = GridLayout(cols=2, spacing=15, size_hint_y=0.8)

        for cat in categories:
            btn = Button(text=cat, font_size='20sp')
            btn.bind(on_release=lambda inst, c=cat: self.go_to_category(c))
            grid.add_widget(btn)

        layout.add_widget(title)
        layout.add_widget(grid)
        self.add_widget(layout)

    def go_to_category(self, category_name):
        self.manager.current = category_name


class CategoryScreen(Screen):
    SWIPE_THRESHOLD = 50

    def __init__(self, name, image_paths, **kwargs):
        super().__init__(name=name, **kwargs)
        self.image_paths = image_paths
        self.current_index = 0
        self._touch_start = None

        # Root layout
        root = BoxLayout(orientation='vertical')

        # Display pane (80%)
        self.img = Image(size_hint_y=0.8, allow_stretch=True, keep_ratio=True)
        root.add_widget(self.img)

        # Instruction pane (20%)
        instr = BoxLayout(size_hint_y=0.2, spacing=20, padding=[20,10])
        left_btn  = Button(text='⟵', font_size='24sp')
        up_btn    = Button(text='⟷', font_size='24sp')  # we'll override ⟷ to ↑ below
        right_btn = Button(text='⟶', font_size='24sp')

        left_btn.bind(on_release=lambda *a: self.show_previous())
        right_btn.bind(on_release=lambda *a: self.show_next())
        up_btn.bind(on_release=lambda *a: self.manager.switch_to(self.manager.get_screen('main')))

        instr.add_widget(left_btn)
        instr.add_widget(up_btn)
        instr.add_widget(right_btn)
        root.add_widget(instr)

        self.add_widget(root)
        self.show_image()

    def show_image(self):
        self.img.source = self.image_paths[self.current_index]

    def show_next(self):
        if self.current_index < len(self.image_paths) - 1:
            self.current_index += 1
            self.show_image()

    def show_previous(self):
        if self.current_index > 0:
            self.current_index -= 1
            self.show_image()

    def on_touch_down(self, touch):
        self._touch_start = touch.pos
        return super().on_touch_down(touch)

    def on_touch_up(self, touch):
        if not self._touch_start:
            return super().on_touch_up(touch)

        dx = touch.pos[0] - self._touch_start[0]
        dy = touch.pos[1] - self._touch_start[1]

        # Horizontal swipe
        if abs(dx) > abs(dy) and abs(dx) > self.SWIPE_THRESHOLD:
            if dx < 0:
                self.show_next()
            else:
                self.show_previous()
        # Vertical swipe up
        elif abs(dy) > abs(dx) and dy > self.SWIPE_THRESHOLD:
            self.manager.current = 'main'

        self._touch_start = None
        return super().on_touch_up(touch)


class TransportApp(App):
    def build(self):
        categories = load_category_images().keys()
        sm = ScreenManager()

        # Main menu
        sm.add_widget(MainMenuScreen(categories, name='main'))

        # One screen per category
        for cat, paths in load_category_images().items():
            sm.add_widget(CategoryScreen(name=cat, image_paths=paths))

        Window.clearcolor = (1, 1, 1, 1)
        return sm


if __name__ == '__main__':
    TransportApp().run()