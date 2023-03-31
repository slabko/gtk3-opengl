#include <stdbool.h>

#include <GL/gl.h>
#include <gtk/gtk.h>

#include "background.h"
#include "matrix.h"
#include "model.h"
#include "program.h"
#include "util.h"
#include "view.h"

static gboolean panning = FALSE;

static void
on_resize(GtkGLArea *area, gint width, gint height) {
    view_set_window(width, height);
    background_set_window(width, height);
}

static gboolean
on_render(GtkGLArea *glarea, GdkGLContext *context) {
    // Clear canvas:
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Draw background:
    background_draw();

    // Draw model:
    model_draw();

    // Don't propagate signal:
    return TRUE;
}

static void
on_realize(GtkGLArea *glarea) {
    // Make current:
    gtk_gl_area_make_current(glarea);


    // Print version info:
    const GLubyte *renderer = glGetString(GL_RENDERER);
    const GLubyte *version = glGetString(GL_VERSION);
    printf("OpenGL Renderer: %s\n", renderer);
    printf("OpenGL version supported %s\n", version);

    // Enable depth buffer:
    gtk_gl_area_set_has_depth_buffer(glarea, TRUE);

    // Init programs:
    programs_init();

    // Init background:
    background_init();

    // Init model:
    model_init();

    // Get frame clock:
#if GTK_CHECK_VERSION(4, 0, 0)
    GdkFrameClock *frame_clock = gtk_widget_get_frame_clock(GTK_WIDGET(glarea));
    g_signal_connect_swapped(frame_clock, "update", G_CALLBACK(gtk_gl_area_queue_render), glarea);
    gdk_frame_clock_begin_updating(frame_clock);
#else
    GdkGLContext *glcontext = gtk_gl_area_get_context(glarea);
    GdkWindow *glwindow = gdk_gl_context_get_window(glcontext);
    GdkFrameClock *frame_clock = gdk_window_get_frame_clock(glwindow);

    // Connect update signal:
    g_signal_connect_swapped(frame_clock, "update", G_CALLBACK(gtk_gl_area_queue_render), glarea);

    // Start updating:
    gdk_frame_clock_begin_updating(frame_clock);
#endif

}

static void
connect_glarea_signals(GtkWidget *glarea) {
    g_signal_connect(glarea, "realize", G_CALLBACK(on_realize), NULL);
    g_signal_connect(glarea, "render", G_CALLBACK(on_render), NULL);
    g_signal_connect(glarea, "resize", G_CALLBACK(on_resize), NULL);
}

void
gui_activate(GtkApplication *app) {
    // Create toplevel window, add GtkGLArea:
    GtkWidget *window = gtk_application_window_new(app);

    GtkWidget *glarea = gtk_gl_area_new();
    gtk_gl_area_set_required_version(GTK_GL_AREA(glarea), 4, 6);
    connect_glarea_signals(glarea);

#if GTK_CHECK_VERSION(4, 0, 0)
    gtk_window_set_child(GTK_WINDOW(window), glarea);
    gtk_window_present(GTK_WINDOW(window));
#else
    gtk_container_add(GTK_CONTAINER(window), glarea);
    gtk_widget_show_all(window);
#endif
}

int
gui_run(int argc, char **argv) {
    g_autoptr(GtkApplication) app = gtk_application_new("org.example.opengl", G_APPLICATION_DEFAULT_FLAGS);
    g_signal_connect(app, "activate", G_CALLBACK(gui_activate), NULL);

    return g_application_run(G_APPLICATION(app), argc, argv);
}
