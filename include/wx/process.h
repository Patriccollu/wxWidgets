/////////////////////////////////////////////////////////////////////////////
// Name:        wx/process.h
// Purpose:     wxProcess class
// Author:      Guilhem Lavaux
// Modified by: Vadim Zeitlin to check error codes, added Detach() method
// Created:     24/06/98
// Copyright:   (c) 1998 Guilhem Lavaux
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////

#ifndef _WX_PROCESSH__
#define _WX_PROCESSH__

#include "wx/event.h"

#if wxUSE_STREAMS
    #include "wx/stream.h"
#endif

#include "wx/utils.h"       // for wxSignal

// the wxProcess creation flags
enum
{
    // no redirection
    wxPROCESS_DEFAULT = 0,

    // redirect the IO of the child process
    wxPROCESS_REDIRECT = 1
};

// ----------------------------------------------------------------------------
// A wxProcess object should be passed to wxExecute - than its OnTerminate()
// function will be called when the process terminates.
// ----------------------------------------------------------------------------

class WXDLLIMPEXP_BASE wxProcess : public wxEvtHandler
{
public:
    // kill the process with the given PID
    static wxKillError Kill(int pid, wxSignal sig = wxSIGTERM, int flags = wxKILL_NOCHILDREN);

    // test if the given process exists
    static bool Exists(int pid);

    // this function replaces the standard popen() one: it launches a process
    // asynchronously and allows the caller to get the streams connected to its
    // std{in|out|err}
    //
    // on error nullptr is returned, in any case the process object will be
    // deleted automatically when the process terminates and should *not* be
    // deleted by the caller
    static wxProcess *Open(const wxString& cmd, int flags = wxEXEC_ASYNC);


    // ctors
    wxProcess(wxEvtHandler *parent = nullptr, int nId = wxID_ANY)
        { Init(parent, nId, wxPROCESS_DEFAULT); }

    wxProcess(int flags) { Init(nullptr, wxID_ANY, flags); }

    virtual ~wxProcess();

    // get the process ID of the process executed by Open()
    long GetPid() const { return m_pid; }

    // may be overridden to be notified about process termination
    virtual void OnTerminate(int pid, int status);

    // call this before passing the object to wxExecute() to redirect the
    // launched process stdin/stdout, then use GetInputStream() and
    // GetOutputStream() to get access to them
    void Redirect() { m_redirect = true; }
    bool IsRedirected() const { return m_redirect; }

    // detach from the parent - should be called by the parent if it's deleted
    // before the process it started terminates
    void Detach();

    // Activates a GUI process by bringing its (main) window to the front.
    //
    // Currently only implemented in wxMSW, simply returns false under the
    // other platforms.
    bool Activate() const;

#if wxUSE_STREAMS
    // Pipe handling
    wxInputStream *GetInputStream() const { return m_inputStream; }
    wxInputStream *GetErrorStream() const { return m_errorStream; }
    wxOutputStream *GetOutputStream() const { return m_outputStream; }

    // close the output stream indicating that nothing more will be written
    void CloseOutput() { delete m_outputStream; m_outputStream = nullptr; }

    // return true if the child process stdout is not closed
    bool IsInputOpened() const;

    // return true if any input is available on the child process stdout/err
    bool IsInputAvailable() const;
    bool IsErrorAvailable() const;

    // implementation only (for wxExecute)
    //
    // NB: the streams passed here should correspond to the child process
    //     stdout, stdin and stderr and here the normal naming convention is
    //     used unlike elsewhere in this class
    void SetPipeStreams(wxInputStream *outStream,
                        wxOutputStream *inStream,
                        wxInputStream *errStream);
#endif // wxUSE_STREAMS

    // priority
        // Sets the priority to the given value: see wxPRIORITY_XXX constants.
        //
        // NB: the priority can only be set before the process is created
    void SetPriority(unsigned priority);

        // Get the current priority.
    unsigned GetPriority() const { return m_priority; }

    // implementation only - don't use!
    // --------------------------------

    // needs to be public since it needs to be used from wxExecute() global func
    void SetPid(long pid) { m_pid = pid; }

protected:
    void Init(wxEvtHandler *parent, int id, int flags);

    int m_id;
    long m_pid;

    unsigned m_priority;

#if wxUSE_STREAMS
    // these streams are connected to stdout, stderr and stdin of the child
    // process respectively (yes, m_inputStream corresponds to stdout -- very
    // confusing but too late to change now)
    wxInputStream  *m_inputStream,
                   *m_errorStream;
    wxOutputStream *m_outputStream;
#endif // wxUSE_STREAMS

    bool m_redirect;

    wxDECLARE_DYNAMIC_CLASS(wxProcess);
    wxDECLARE_NO_COPY_CLASS(wxProcess);
};

// ----------------------------------------------------------------------------
// wxProcess events
// ----------------------------------------------------------------------------

class WXDLLIMPEXP_FWD_BASE wxProcessEvent;

wxDECLARE_EXPORTED_EVENT( WXDLLIMPEXP_BASE, wxEVT_END_PROCESS, wxProcessEvent );

class WXDLLIMPEXP_BASE wxProcessEvent : public wxEvent
{
public:
    wxProcessEvent(int nId = 0, int pid = 0, int exitcode = 0) : wxEvent(nId)
    {
        m_eventType = wxEVT_END_PROCESS;
        m_pid = pid;
        m_exitcode = exitcode;
    }

    // accessors
        // PID of process which terminated
    int GetPid() const { return m_pid; }

        // the exit code
    int GetExitCode() const { return m_exitcode; }

    // implement the base class pure virtual
    wxNODISCARD virtual wxEvent *Clone() const override { return new wxProcessEvent(*this); }

public:
    int m_pid,
        m_exitcode;

    wxDECLARE_DYNAMIC_CLASS_NO_ASSIGN_DEF_COPY(wxProcessEvent);
};

typedef void (wxEvtHandler::*wxProcessEventFunction)(wxProcessEvent&);

#define wxProcessEventHandler(func) \
    wxEVENT_HANDLER_CAST(wxProcessEventFunction, func)

#define EVT_END_PROCESS(id, func) \
   wx__DECLARE_EVT1(wxEVT_END_PROCESS, id, wxProcessEventHandler(func))

#endif // _WX_PROCESSH__
