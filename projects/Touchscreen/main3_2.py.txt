import os
from kivy.app import App
from kivy.uix.boxlayout import BoxLayout
from kivy.uix.gridlayout import GridLayout
from kivy.uix.button import Button
from kivy.uix.image import Image
from kivy.uix.screenmanager import ScreenManager, Screen
from kivy.properties import ObjectProperty, ListProperty, NumericProperty
from kivy.core.window import Window
from kivy.vector import Vector

class MainScreen(Screen):
    """
    The main screen of the application, displaying category buttons.
    """
    # Object properties to store references to image lists
    cars_images = ListProperty([])
    motorcycles_images = ListProperty([])
    planes_images = ListProperty([])
    ships_images = ListProperty([])

    def __init__(self, **kwargs):
        super().__init__(**kwargs)
        self.name = 'main_screen'
        self.load_images()
        self.create_layout()

    def load_images(self):
        """
        Loads image file paths from the 'media' directory and its subdirectories.
        Sorts the image lists alphabetically.
        """
        base_dir = "media"
        categories = ["Cars", "Motorcycles", "Planes", "Ships"]

        for category in categories:
            path = os.path.join(base_dir, category)
            if os.path.exists(path) and os.path.isdir(path):
                image_files = sorted([
                    os.path.join(path, f)
                    for f in os.listdir(path)
                    if f.lower().endswith(('.png', '.jpg', '.jpeg', '.gif', '.bmp'))
                ])
                if category == "Cars":
                    self.cars_images = image_files
                elif category == "Motorcycles":
                    self.motorcycles_images = image_files
                elif category == "Planes":
                    self.planes_images = image_files
                elif category == "Ships":
                    self.ships_images = image_files
            else:
                print(f"Warning: Directory '{path}' not found. Please create it and add images.")

    def create_layout(self):
        """
        Creates the grid layout for the category buttons.
        """
        layout = GridLayout(cols=2, spacing=20, padding=20)
        
        # Create buttons for each category
        btn_cars = Button(text="Cars", on_release=self.go_to_image_display)
        btn_motorcycles = Button(text="Motorcycles", on_release=self.go_to_image_display)
        btn_planes = Button(text="Planes", on_release=self.go_to_image_display)
        btn_ships = Button(text="Ships", on_release=self.go_to_image_display)

        layout.add_widget(btn_cars)
        layout.add_widget(btn_motorcycles)
        layout.add_widget(btn_planes)
        layout.add_widget(btn_ships)

        self.add_widget(layout)

    def go_to_image_display(self, instance):
        """
        Transitions to the ImageDisplayScreen with the selected category's images.
        """
        app = App.get_running_app()
        image_display_screen = app.screen_manager.get_screen('image_display_screen')

        # Set the images list based on the button clicked
        if instance.text == "Cars":
            image_display_screen.image_list = self.cars_images
        elif instance.text == "Motorcycles":
            image_display_screen.image_list = self.motorcycles_images
        elif instance.text == "Planes":
            image_display_screen.image_list = self.planes_images
        elif instance.text == "Ships":
            image_display_screen.image_list = self.ships_images
        
        # Reset image index and update the display
        image_display_screen.current_image_index = 0
        image_display_screen.update_image()
        
        app.screen_manager.current = 'image_display_screen'

class ImageDisplayScreen(Screen):
    """
    Displays images and provides navigation controls.
    Supports button clicks, swipe, and mouse drag gestures for navigation.
    """
    image_list = ListProperty([])
    current_image_index = NumericProperty(0)
    image_widget = ObjectProperty(None) # Reference to the Kivy Image widget

    # For swipe/drag detection
    _touch_start_pos = None
    _touch_start_time = None
    _is_dragging = False

    def __init__(self, **kwargs):
        super().__init__(**kwargs)
        self.name = 'image_display_screen'
        self.create_layout()

        # Bind touch events for swipe/drag navigation
        # IMPORTANT: When binding to Window events, the first argument received by the
        # bound method is the Window instance itself, followed by the touch object.
        Window.bind(on_touch_down=self.on_touch_down_event)
        Window.bind(on_touch_up=self.on_touch_up_event)
        Window.bind(on_touch_move=self.on_touch_move_event)

    def create_layout(self):
        """
        Creates the layout for the image display and instruction pane.
        """
        main_layout = BoxLayout(orientation='vertical')

        # Display Pane (90% height)
        self.image_widget = Image(source='', allow_stretch=True, keep_ratio=True)
        display_pane = BoxLayout(size_hint_y=0.9)
        display_pane.add_widget(self.image_widget)
        main_layout.add_widget(display_pane)

        # Instruction Pane (10% height)
        instruction_pane = BoxLayout(size_hint_y=0.1, padding=10, spacing=20)
        
        # Left arrow button for previous image
        btn_prev = Button(text="<", font_size=30, on_release=self.show_previous_image)
        instruction_pane.add_widget(btn_prev)

        # Up arrow button to return to Main Menu
        btn_main_menu = Button(text="^", font_size=30, on_release=self.go_to_main_menu)
        instruction_pane.add_widget(btn_main_menu)

        # Right arrow button for next image
        btn_next = Button(text=">", font_size=30, on_release=self.show_next_image)
        instruction_pane.add_widget(btn_next)
        
        main_layout.add_widget(instruction_pane)
        self.add_widget(main_layout)

    def update_image(self):
        """
        Updates the displayed image based on current_image_index.
        Handles cases where the image_list is empty.
        """
        if self.image_list:
            if 0 <= self.current_image_index < len(self.image_list):
                self.image_widget.source = self.image_list[self.current_image_index]
            else:
                # Loop around if index goes out of bounds
                self.current_image_index = self.current_image_index % len(self.image_list)
                self.image_widget.source = self.image_list[self.current_image_index]
        else:
            self.image_widget.source = '' # Clear image if no list or empty list
            print("No images to display for this category.")

    def show_next_image(self, *args):
        """
        Increments the image index and updates the displayed image.
        Loops to the beginning if at the end of the list.
        """
        if self.image_list:
            self.current_image_index = (self.current_image_index + 1) % len(self.image_list)
            self.update_image()

    def show_previous_image(self, *args):
        """
        Decrements the image index and updates the displayed image.
        Loops to the end if at the beginning of the list.
        """
        if self.image_list:
            self.current_image_index = (self.current_image_index - 1 + len(self.image_list)) % len(self.image_list)
            self.update_image()

    def go_to_main_menu(self, *args):
        """
        Transitions back to the MainScreen.
        """
        App.get_running_app().screen_manager.current = 'main_screen'

    # Corrected method signatures to accept 'instance' (the Window object)
    def on_touch_down_event(self, instance, touch):
        """
        Captures the starting position and time of a touch/mouse press.
        Only active when this screen is the current one.
        """
        if App.get_running_app().screen_manager.current == self.name:
            self._touch_start_pos = Vector(touch.x, touch.y)
            self._touch_start_time = touch.time_start
            self._is_dragging = False # Reset drag flag
        return super().on_touch_down(touch)

    def on_touch_move_event(self, instance, touch):
        """
        Detects if a drag is occurring.
        Only active when this screen is the current one.
        """
        if App.get_running_app().screen_manager.current == self.name and self._touch_start_pos:
            # Consider it a drag if moved more than a threshold
            if (Vector(touch.x, touch.y) - self._touch_start_pos).length() > 20: # 20 pixels threshold
                self._is_dragging = True
        return super().on_touch_move(touch)

    def on_touch_up_event(self, instance, touch):
        """
        Detects swipe or drag gestures for navigation.
        Only active when this screen is the current one.
        """
        if App.get_running_app().screen_manager.current == self.name and self._touch_start_pos:
            end_pos = Vector(touch.x, touch.y)
            # Only process if it was a drag gesture
            if self._is_dragging:
                # Calculate the difference vector
                diff = end_pos - self._touch_start_pos

                # Determine direction of swipe/drag
                if abs(diff.x) > abs(diff.y): # Horizontal drag
                    if diff.x < -50: # Swiped left (next image)
                        self.show_next_image()
                    elif diff.x > 50: # Swiped right (previous image)
                        self.show_previous_image()
                else: # Vertical drag
                    if diff.y > 50: # Swiped up (return to main menu)
                        self.go_to_main_menu()
            
            # Reset touch state
            self._touch_start_pos = None
            self._touch_start_time = None
            self._is_dragging = False

        return super().on_touch_up(touch)

class SlideshowApp(App):
    """
    Main application class for the Kivy slideshow.
    """
    screen_manager = ObjectProperty(None)

    def build(self):
        """
        Builds the Kivy application by setting up the screen manager
        and adding the main and image display screens.
        """
        self.title = "Main Window" # Set the initial window title

        self.screen_manager = ScreenManager()
        self.screen_manager.add_widget(MainScreen(name='main_screen'))
        self.screen_manager.add_widget(ImageDisplayScreen(name='image_display_screen'))

        return self.screen_manager

if __name__ == '__main__':
    # Ensure the 'media' directory and its subdirectories exist for testing
    if not os.path.exists("media"):
        os.makedirs("media")
    for category in ["Cars", "Motorcycles", "Planes", "Ships"]:
        path = os.path.join("media", category)
        if not os.path.exists(path):
            os.makedirs(path)
            print(f"Created dummy directory: {path}. Please add images here.")
            # Create a simple dummy image for demonstration if directories are empty
            dummy_image_path = os.path.join(path, f"dummy_{category}.png")
            if not os.path.exists(dummy_image_path):
                # Create a simple dummy image using PIL if available, otherwise just print a message
                try:
                    from PIL import Image as PilImage, ImageDraw
                    img = PilImage.new('RGB', (600, 400), color = (73, 109, 137))
                    d = ImageDraw.Draw(img)
                    d.text((10,10), f"No {category} Images Yet", fill=(255,255,0))
                    img.save(dummy_image_path)
                    print(f"Created dummy image: {dummy_image_path}")
                except ImportError:
                    print(f"PIL not found. Cannot create dummy image for {path}. Please add your own images.")
    
    SlideshowApp().run()
