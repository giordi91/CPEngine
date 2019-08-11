
#if CP_LINUX_PLATFORM
#include "CPEngine/platform/linux/core/linuxWindow.h"

#include "CPEngine/core/logging.h"

#include "CPEngine/core/core.h"
#include "CPEngine/core/events/keyboardEvent.h"
#include "CPEngine/core/events/mouseEvent.h"

#include <xcb/xcb.h>
/*
#include<stdio.h>
#include<stdlib.h>
#include<X11/X.h>
#include<X11/Xlib.h>
#include<X11/Xutil.h>
#include<GL/gl.h>
#include<GL/glx.h>
 */
namespace cp::linux{

static inline xcb_intern_atom_reply_t* intern_atom_helper(xcb_connection_t *conn, bool only_if_exists, const char *str)
{
    xcb_intern_atom_cookie_t cookie = xcb_intern_atom(conn, only_if_exists, strlen(str), str);
    return xcb_intern_atom_reply(conn, cookie, NULL);
}
LinuxWindow::LinuxWindow(const core::WindowProps &props) {

  m_data = props;
  logCoreInfo("Creating LinuxWindow with dimensions: {0}x{1}", props.width,
              props.height);

  /*
  //X11 is a sort of networked graphics api, meaning you can send graphics
  //windows stuff over the network to another x11, passing NULL tells
  //we are going to use this machine
  Window                  root;
  GLint                   att[] = { GLX_RGBA, GLX_DEPTH_SIZE, 24, GLX_DOUBLEBUFFER, None };
  XVisualInfo             *vi;
  Colormap                cmap;
  XSetWindowAttributes    swa;
  Window                  win;
  GLXContext              glc;
  XWindowAttributes       gwa;
  XEvent                  xev;

        Display* dpy = XOpenDisplay(NULL);

  if(dpy == NULL) {
      printf("\n\tcannot connect to X server\n\n");
      exit(0);
  }
  //getting a handle to the root window
  //this is the desktop background window
  root = DefaultRootWindow(dpy);



  //let us define what the window will use,in this case the components, depth and
  //that is double buffered
  vi = glXChooseVisual(dpy, 0, att);

  if(vi == NULL) {
      printf("\n\tno appropriate visual found\n\n");
      exit(0);
  }
  else {
      printf("\n\tvisual %p selected\n", (void *)vi->visualid);
  }

  //this is the color map for the window, based on the visual we decided to use
  cmap = XCreateColormap(dpy, root, vi->visual, AllocNone);

  swa.colormap = cmap;
  swa.event_mask = ExposureMask | KeyPressMask;

  win = XCreateWindow(dpy, root, 0, 0, props.width, props.height, 0, vi->depth, InputOutput, vi->visual, CWColormap | CWEventMask, &swa);
  //lets make the window appear;
  XMapWindow(dpy, win);
  XStoreName(dpy, win, props.title);
        glc = glXCreateContext(dpy, vi, NULL, GL_TRUE);
        glXMakeCurrent(dpy, win, glc);
        */


    const xcb_setup_t *setup;
    xcb_screen_iterator_t iter;
    int scr;

    connection = xcb_connect(NULL, &scr);
    if (connection == NULL) {
        printf("Could not find a compatible Vulkan ICD!\n");
        fflush(stdout);
        exit(1);
    }

    setup = xcb_get_setup(connection);
    iter = xcb_setup_roots_iterator(setup);
    while (scr-- > 0)
        xcb_screen_next(&iter);
    screen = iter.data;

        uint32_t value_mask, value_list[32];

        window = xcb_generate_id(connection);

        value_mask = XCB_CW_BACK_PIXEL | XCB_CW_EVENT_MASK;
        value_list[0] = screen->black_pixel;
        value_list[1] =
                XCB_EVENT_MASK_KEY_RELEASE |
                XCB_EVENT_MASK_KEY_PRESS |
                XCB_EVENT_MASK_EXPOSURE |
                XCB_EVENT_MASK_STRUCTURE_NOTIFY |
                XCB_EVENT_MASK_POINTER_MOTION |
                XCB_EVENT_MASK_BUTTON_PRESS |
                XCB_EVENT_MASK_BUTTON_RELEASE;


        xcb_create_window(connection,
                          XCB_COPY_FROM_PARENT,
                          window, screen->root,
                          0, 0, props.width, props.height, 0,
                          XCB_WINDOW_CLASS_INPUT_OUTPUT,
                          screen->root_visual,
                          value_mask, value_list);

        /* Magic code that will send notification when window is destroyed */
        xcb_intern_atom_reply_t* reply = intern_atom_helper(connection, true, "WM_PROTOCOLS");
        atom_wm_delete_window = intern_atom_helper(connection, false, "WM_DELETE_WINDOW");

        xcb_change_property(connection, XCB_PROP_MODE_REPLACE,
                            window, (*reply).atom, 4, 32, 1,
                            &(*atom_wm_delete_window).atom);

        std::string windowTitle = props.title;
        xcb_change_property(connection, XCB_PROP_MODE_REPLACE,
                            window, XCB_ATOM_WM_NAME, XCB_ATOM_STRING, 8,
                            strlen(props.title), windowTitle.c_str());

        free(reply);

        //if (settings.fullscreen)
        //{
        //    xcb_intern_atom_reply_t *atom_wm_state = intern_atom_helper(connection, false, "_NET_WM_STATE");
        //    xcb_intern_atom_reply_t *atom_wm_fullscreen = intern_atom_helper(connection, false, "_NET_WM_STATE_FULLSCREEN");
        //    xcb_change_property(connection,
        //                        XCB_PROP_MODE_REPLACE,
        //                        window, atom_wm_state->atom,
        //                        XCB_ATOM_ATOM, 32, 1,
        //                        &(atom_wm_fullscreen->atom));
        //    free(atom_wm_fullscreen);
        //    free(atom_wm_state);
        //}

        xcb_map_window(connection, window);

}
void LinuxWindow::handleEvent(const xcb_generic_event_t *event)
{
  #define ASSERT_CALLBACK_AND_DISPATCH(e)                                        \
  assert(m_callback != nullptr);                                                 \
  m_callback(e);
    switch (event->response_type & 0x7f)
    {
        case XCB_CLIENT_MESSAGE: {

            if ((*(xcb_client_message_event_t *) event).data.data32[0] ==
                (*atom_wm_delete_window).atom) {
                core::WindowCloseEvent closeEvent;
                ASSERT_CALLBACK_AND_DISPATCH(closeEvent);
            }
            break;
        }
        case XCB_MOTION_NOTIFY:
        {
            xcb_motion_notify_event_t *motion = (xcb_motion_notify_event_t *)event;
            core::MouseMoveEvent e{(float)motion->event_x, (float)motion->event_y};
            ASSERT_CALLBACK_AND_DISPATCH(e);
            break;
        }
        case XCB_BUTTON_PRESS:
        {
            xcb_button_press_event_t *press = (xcb_button_press_event_t *)event;
            if (press->detail == XCB_BUTTON_INDEX_1) {
                core::MouseButtonPressEvent e{core::MOUSE_BUTTONS_EVENT::LEFT};
                ASSERT_CALLBACK_AND_DISPATCH(e);
            }
            if (press->detail == XCB_BUTTON_INDEX_2)
            {
                core::MouseButtonPressEvent e{core::MOUSE_BUTTONS_EVENT::MIDDLE};
                ASSERT_CALLBACK_AND_DISPATCH(e);
            }
            if (press->detail == XCB_BUTTON_INDEX_3) {
                core::MouseButtonPressEvent e{core::MOUSE_BUTTONS_EVENT::RIGHT};
                ASSERT_CALLBACK_AND_DISPATCH(e);
            }
        }
            break;
        case XCB_BUTTON_RELEASE:
        {
            xcb_button_press_event_t *press = (xcb_button_press_event_t *)event;
            if (press->detail == XCB_BUTTON_INDEX_1) {
                core::MouseButtonReleaseEvent e{core::MOUSE_BUTTONS_EVENT::LEFT};
                ASSERT_CALLBACK_AND_DISPATCH(e);
            }
            if (press->detail == XCB_BUTTON_INDEX_2)
            {
                core::MouseButtonReleaseEvent e{core::MOUSE_BUTTONS_EVENT::MIDDLE};
                ASSERT_CALLBACK_AND_DISPATCH(e);
            }
            if (press->detail == XCB_BUTTON_INDEX_3) {
                core::MouseButtonReleaseEvent e{core::MOUSE_BUTTONS_EVENT::RIGHT};
                ASSERT_CALLBACK_AND_DISPATCH(e);
            }
        }
            break;
        case XCB_KEY_PRESS:
        {
            const xcb_key_release_event_t *keyEvent = (const xcb_key_release_event_t *)event;
            core::KeyboardPressEvent e{keyEvent->detail};
            ASSERT_CALLBACK_AND_DISPATCH(e);
            break;
        }
        case XCB_KEY_RELEASE:
        {

            const xcb_key_release_event_t *keyEvent = (const xcb_key_release_event_t *)event;
            #ifdef QUIT_ESCAPE
                // here we hard-coded this behavior where if the VK_ESCAPE button
                // is pressed I want the message to be sent out as close window,
                // this is a personal preference
                if(keyEvent->detail == 0x9)
                {
                    core::WindowCloseEvent closeEvent;
                    ASSERT_CALLBACK_AND_DISPATCH(closeEvent);
                    break;
                }
            #endif
            break;
        }
        case XCB_DESTROY_NOTIFY:
            quit = true;
            break;
        case XCB_CONFIGURE_NOTIFY:
        {
            const xcb_configure_notify_event_t *cfgEvent = (const xcb_configure_notify_event_t *)event;
            //(prepared) &&
            if ( ((cfgEvent->width != m_data.width) || (cfgEvent->height != m_data.height)))
            {
                uint32_t w= cfgEvent->width;
                uint32_t h= cfgEvent->height;
                core::WindowResizeEvent resizeEvent{w, h};
                m_callback(resizeEvent);
            }
            break;
        }
        default:
            break;
    }
}

void LinuxWindow::onUpdate() {
    //XNextEvent(dpy, &xev);
    xcb_flush(connection);
    //while (!quit)
    //{
        xcb_generic_event_t *event;
        while ((event = xcb_poll_for_event(connection)))
        {
            handleEvent(event);
            free(event);
        }
}

void LinuxWindow::onResize(unsigned int width, unsigned int height) {
  m_data.width = width;
  m_data.height = height;
}

unsigned int LinuxWindow::getWidth() const { return m_data.width; }
unsigned int LinuxWindow::getHeight() const { return m_data.height; }
void LinuxWindow::setEventCallback(const EventCallbackFn &callback) {
  m_callback = callback;
}

} // namespace cp::linux
#endif