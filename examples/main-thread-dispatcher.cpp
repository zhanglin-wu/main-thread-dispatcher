/**
 * main-thread-dispatcher.cpp
 * 
 * https://developer.gnome.org/glibmm/2.64/thread_2dispatcher_8cc-example.html
 * 
 * Glib::Dispatcher example -- cross thread signalling
 * by Daniel Elstner <daniel.kitta@gmail.com>
 * modified to only use glibmm
 * by J. Abelardo Gutierrez <jabelardo@cantv.net>
 * 
 * Build command:
 *   g++ -g -Wall -o main-thread-dispatcher main-thread-dispatcher.cpp `pkg-config --cflags --libs glibmm-2.4 giomm-2.4` && ./main-thread-dispatcher
 *
 * Copyright (c) 2002-2003 Free Software Foundation
 */

#include <algorithm>
#include <functional>
#include <iostream>
#include <thread>
#include <vector>

#include <glibmm.h>

namespace
{
    /**
     * Note that it does not make sense for this class to inherit from
     * sigc::trackable, as doing so would only give a false sense of security.
     * Once the thread launch has been triggered, the object has to stay alive
     * until the thread has been joined again. The code running in the thread
     * assumes the existence of the object. If it is destroyed earlier, the
     * program will crash, with sigc::trackable or without it.
     */
    class ThreadProgress
    {
    public:

        explicit ThreadProgress(int the_id);
        ~ThreadProgress();

        int id() const;

        void launch();
        void join();

        bool unfinished() const;
        sigc::signal<void>& signal_finished();

    private:
        
        void progress_increment();
        void thread_function();

    private:

        enum
        {
            ITERATIONS = 10
        };

        // Note that the thread does not write to the member data at all. It only
        // reads signal_increment_, which is only written to before the thread is
        // launched. Therefore, no locking is required.
        std::thread* thread_;
        int id_;

        unsigned int progress_;

        Glib::Dispatcher signal_increment_;
        sigc::signal<void> signal_finished_;
    };

    class Application : public sigc::trackable
    {
    public:

        Application();
        ~Application();
        void run();

    private:

        void launch_threads();
        void on_progress_finished(ThreadProgress* thread_progress);

    private:

        Glib::RefPtr<Glib::MainLoop> main_loop_;
        std::vector<ThreadProgress*> progress_threads_;
    };

    template <class T>
    class DeletePtr
    {
        typedef void argument_type;
        typedef T result_type;

        public:

            void operator()(T ptr) const { delete ptr; }
    };

    ThreadProgress::ThreadProgress(int the_id) :
        thread_(nullptr),
        id_(the_id),
        progress_(0)
    {
        // Connect to the cross-thread signal
        signal_increment_.connect(sigc::mem_fun(*this, &ThreadProgress::progress_increment));
    }

    ThreadProgress::~ThreadProgress()
    {
        // It is an error if the thread is still running at this point
        g_return_if_fail(thread_ == nullptr);
    }

    int ThreadProgress::id() const
    {
        return id_;
    }

    void ThreadProgress::launch()
    {
        // Create a joinable thread
        thread_ = new std::thread([this]() {
            this->thread_function();
        });

        std::cout << "Logic thread " << id_ << " created as std::thread " << thread_->get_id() << std::endl;
    }

    void ThreadProgress::join()
    {
        thread_->join();

        delete thread_;
        thread_ = nullptr;
    }

    bool ThreadProgress::unfinished() const
    {
        return (progress_ < ITERATIONS);
    }

    sigc::signal<void>& ThreadProgress::signal_finished()
    {
        return signal_finished_;
    }

    void ThreadProgress::progress_increment()
    {
        ++progress_;
        std::cout << "Logical thread " << id_ << ": " << progress_ << '%' << " | Signal received by std::thread " << std::this_thread::get_id() << std::endl;

        if (progress_ >= ITERATIONS)
            signal_finished_();
    }

    void ThreadProgress::thread_function()
    {
        Glib::Rand rand;
        for (auto i = 0; i < ITERATIONS; ++i)
        {
            Glib::usleep(rand.get_int_range(2000, 20000));

            std::cout << "Logical thread " << id_ << ": " << (i + 1) << '%' << " | Signal emitted from std::thread " << std::this_thread::get_id() << std::endl;

            // Tell the main thread to increment the progress value
            signal_increment_();
        }
    }

    Application::Application() :
        main_loop_(Glib::MainLoop::create()),
        progress_threads_(4)
    {
        try
        {
            for (std::vector<ThreadProgress*>::size_type i = 0; i < progress_threads_.size(); ++i)
            {
                ThreadProgress* const progress = new ThreadProgress(i + 1);
                progress_threads_[i] = progress;
                progress->signal_finished().connect(
                    sigc::bind(sigc::mem_fun(*this, &Application::on_progress_finished), progress));
            }
        }
        catch (...)
        {
            // In your own code, you should preferably use a smart pointer
            // to ensure exception safety
            std::for_each(progress_threads_.begin(), progress_threads_.end(), DeletePtr<ThreadProgress*>());
            
            throw;
        }
    }

    Application::~Application()
    {
        std::for_each(progress_threads_.begin(), progress_threads_.end(), DeletePtr<ThreadProgress*>());
    }

    void Application::run()
    {
        // Case 1. Use the idle signal
        // Install a one-shot idle handler to launch the threads
        // Glib::signal_idle().connect_once(sigc::mem_fun(*this, &Application::launch_threads));
        // main_loop_->get_context()->signal_idle().connect_once(sigc::mem_fun(*this, &Application::launch_threads));

        // Case 2. Use the timeout signal
        // Install a one-shot timeout handler to launch the threads
        // Glib::signal_timeout().connect_once(sigc::mem_fun(*this, &Application::launch_threads), 3000);
        main_loop_->get_context()->signal_timeout().connect_once(sigc::mem_fun(*this, &Application::launch_threads), 3000);

        main_loop_->run();
    }

    void Application::launch_threads()
    {
        std::cout << "Launching " << progress_threads_.size() << " threads:" << std::endl;

        std::for_each(progress_threads_.begin(), progress_threads_.end(), std::mem_fn(&ThreadProgress::launch));

        std::cout << "----------------------------------------" << std::endl;
    }

    void Application::on_progress_finished(ThreadProgress* thread_progress)
    {
        thread_progress->join();
        std::cout << "Thread " << thread_progress->id() << ": finished." << std::endl;
        
        // Quit if it was the last thread to be joined.
        if (std::find_if(progress_threads_.begin(), progress_threads_.end(), std::mem_fn(&ThreadProgress::unfinished)) == progress_threads_.end())
        {
            main_loop_->quit();
        }
    }
} // anonymous namespace

int main(int, char**)
{
    std::cout << "----------------------------------------" << std::endl;
    std::cout << "Main thread: " << std::this_thread::get_id() << std::endl;
    std::cout << "----------------------------------------" << std::endl;

    Glib::init();

    Application application;
    application.run();

    return 0;
}
