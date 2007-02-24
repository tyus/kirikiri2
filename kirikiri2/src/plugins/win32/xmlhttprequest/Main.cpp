/*
 * XMLHttpRequest
 *
 * http://www.w3.org/TR/XMLHttpRequest/
 *
 * Written by Kouhei Yanagita
 *
 */
//---------------------------------------------------------------------------
#include <winsock2.h>
#include <windows.h>
#include <process.h>
#include "tp_stub.h"
#include <sstream>
#include <vector>
#include <map>
#include <algorithm>
#include <boost/regex.hpp>
#pragma comment(lib, "WSock32.lib")
//---------------------------------------------------------------------------



/*
    �v���O�C�����Ńl�C�e�B�u�������ꂽ�N���X��񋟂��A�g���g�����Ŏg�p�ł���悤��
    �����ł��B

    �l�C�e�B�u�N���X�� tTJSNativeInstance ���p�������N���X��ɍ쐬���A���̃l�C
    �e�B�u�N���X�Ƃ̃C���^�[�t�F�[�X�� tTJSNativeClassForPlugin ���x�[�X�ɍ쐬��
    �܂��B

    �uTJS2 ���t�@�����X�v�́u�g�ݍ��݂̎�����v�́u��{�I�Ȏg�����v�ɂ����Ɠ���
    �N���X�������ł͍쐬���܂��B�������A�v���O�C���Ŏ�������s����ATJS2 ���t�@
    �����X�ɂ����Ƃ͎኱�����̎d�����قȂ邱�Ƃɒ��ӂ��Ă��������B
*/



//---------------------------------------------------------------------------
// �e�X�g�N���X
//---------------------------------------------------------------------------
/*
    �e�I�u�W�F�N�g (iTJSDispatch2 �C���^�[�t�F�[�X) �ɂ̓l�C�e�B�u�C���X�^���X��
    �Ă΂��AiTJSNativeInstance �^�̃I�u�W�F�N�g��o�^���邱�Ƃ��ł��A�����
    �I�u�W�F�N�g������o�����Ƃ��ł��܂��B
    �܂��A�l�C�e�B�u�C���X�^���X�̎����ł��B�l�C�e�B�u�C���X�^���X����������ɂ�
    tTJSNativeInstance ����N���X�𓱏o���܂��BtTJSNativeInstance ��
    iTJSNativeInstance �̊�{�I�ȓ�����������Ă��܂��B
*/
class NI_XMLHttpRequest : public tTJSNativeInstance // �l�C�e�B�u�C���X�^���X
{
public:
    NI_XMLHttpRequest()
    {

    }

    tjs_error TJS_INTF_METHOD
        Construct(tjs_int numparams, tTJSVariant **param, iTJSDispatch2 *tjs_obj)
    {
        // TJS2 �I�u�W�F�N�g���쐬�����Ƃ��ɌĂ΂��
        /*
            TJS2 �� new ���Z�q�� TJS2 �I�u�W�F�N�g���쐬�����Ƃ��ɌĂ΂�܂��B
            numparams �� param ������ new ���Z�q�ɓn���ꂽ������\���Ă��܂��B
            tjs_obj �����́A�쐬����� TJS �I�u�W�F�N�g�ł��B
            ���̗�ł́A����������� (����ɂ��ꂪ void �Ŗ������)�A����� Value
            �̏����l�Ƃ��Đݒ肵�Ă��܂��B
        */
        Initialize();
        _target = tjs_obj;

        return S_OK;
    }

    void TJS_INTF_METHOD Invalidate()
    {
        // �I�u�W�F�N�g�������������Ƃ��ɌĂ΂��
        /*
            �I�u�W�F�N�g�������������Ƃ��ɌĂ΂�郁�\�b�h�ł��B�����ɏI������
            �������Ɨǂ��ł��傤�B���̗�ł͉������܂���B
        */
    }


    void Initialize(void) {
        _responseData.clear();
        _responseBody.clear();
        _responseStatus = 0;
        _requestHeaders.clear();
        _aborted = false;
    }

    tjs_int GetReadyState(void) const { return _readyState; }
    void SetReadyState(tjs_int v) { _readyState = v; OnReadyStateChange(); }

    tjs_int GetResponseStatus(void) const {
        RaiseExceptionIfNotResponsed();
        return _responseStatus;
    }

    ttstr GetResponseStatusText(void) const {
        RaiseExceptionIfNotResponsed();

        switch (_responseStatus) {
        case 200: return ttstr("OK");
        case 201: return ttstr("Created");
        case 202: return ttstr("Accepted");
        case 203: return ttstr("Non-Authoritative Information");
        case 204: return ttstr("No Content");
        case 205: return ttstr("Reset Content");
        case 206: return ttstr("Partial Content");
        case 300: return ttstr("Multiple Choices");
        case 301: return ttstr("Moved Permanently");
        case 302: return ttstr("Found");
        case 303: return ttstr("See Other");
        case 304: return ttstr("Not Modified");
        case 305: return ttstr("Use Proxy");
        case 307: return ttstr("Temporary Redirect");
        case 400: return ttstr("Bad Request");
        case 401: return ttstr("Unauthorized");
        case 402: return ttstr("Payment Required");
        case 403: return ttstr("Forbidden");
        case 404: return ttstr("Not Found");
        case 405: return ttstr("Method Not Allowed");
        case 406: return ttstr("Not Acceptable");
        case 407: return ttstr("Proxy Authentication Required");
        case 408: return ttstr("Request Timeout");
        case 409: return ttstr("Conflict");
        case 410: return ttstr("Gone");
        case 411: return ttstr("Length Required");
        case 412: return ttstr("Precondition Failed");
        case 413: return ttstr("Request Entity Too Large");
        case 414: return ttstr("Request-URI Too Long");
        case 415: return ttstr("Unsupported Media Type");
        case 416: return ttstr("Requested Range Not Satisfiable");
        case 417: return ttstr("Expectation Failed");
        case 500: return ttstr("Internal Server Error");
        case 501: return ttstr("Not Implemented");
        case 502: return ttstr("Bad Gateway");
        case 503: return ttstr("Service Unavailable");
        case 504: return ttstr("Gateway Timeout");
        case 505: return ttstr("HTTP Version Not Supported");
        default: return ttstr("");
        }
    }

    const std::vector<char>* GetResponseText(void) const {
        RaiseExceptionIfNotResponsed();

        return &_responseBody;
    }

    void Open(const ttstr &method, const ttstr &uri, bool async, const ttstr &username, const ttstr &password)
    {
        Initialize();

        this->method = method;
        _async = async;

        boost::reg_expression<tjs_char> re(
            ttstr("http://"
                  "("
                  "(?:[a-zA-Z0-9](?:[-a-zA-Z0-9]*[a-zA-Z0-9]|(?:))\\.)*[a-zA-Z](?:[-a-zA-Z0-9]*[a-zA-Z0-9]|(?:))\\.?" // hostname
                  "|"
                  "[0-9]+\\.[0-9]+\\.[0-9]+\\.[0-9]+" // IPv4addr
                  ")"
                  "(?::([0-9]+))?" // port
                  "(.*)").c_str(),
            boost::regbase::normal|boost::regbase::use_except|boost::regbase::nocollate);
        boost::match_results<const tjs_char*> what;
        bool matched = boost::regex_search(uri.c_str(), what, re, boost::match_default);

        if (!matched) {
            TVPThrowExceptionMessage(TJS_W("Wrong URL"));
            return;
        }

        _host = "";
        _host.append(what[1].first, what[1].second);

        if (what[2].matched) {
            _port = TJSStringToInteger(ttstr(what[2].first, what[2].second - what[2].first).c_str());
        }
        else {
            _port = 80;
        }
        
        _path = "";
        _path.append(what[3].first, what[3].second);

        if (IsValidUserInfo(username, password)) {
            std::string authKey = "";
            std::copy(username.c_str(), username.c_str() + username.length(), std::back_inserter(authKey));
            authKey.append(":");
            std::copy(password.c_str(), password.c_str() + password.length(), std::back_inserter(authKey));

            _requestHeaders.insert(std::pair<std::string, std::string>("Authorization", std::string("Basic ") + EncodeBase64(authKey)));
        }
        else {
            TVPThrowExceptionMessage(TJS_W("Wrong UserInfo"));
            return;
        }

        SetReadyState(1);
    }

    void Send()
    {
        if (_hThread) {
            TVPAddLog(ttstr("���X�|���X���߂�O�� send ���܂���"));
            return;
        }

        if (_readyState != 1) {
            TVPThrowExceptionMessage(TJS_W("INVALID_STATE_ERR"));
            return;
        }

        _aborted = false;

        if (_async) {
            _hThread = (HANDLE)_beginthreadex(NULL, 0, StartProc, this, 0, NULL);
        }
        else {
            _Send();
        }
    }

    void _Send(void)
    {
        _responseData.clear();

        WSADATA wsaData;
        if (WSAStartup(MAKEWORD(2, 0), &wsaData)) {
            OnErrorOnSending();
            return;
        }

        SOCKET sock = socket(AF_INET, SOCK_STREAM, 0);
        if (sock == INVALID_SOCKET) {
            OnErrorOnSending();
            return;
        }

        sockaddr_in server;
        server.sin_family = AF_INET;
        server.sin_port = htons(_port);
        server.sin_addr.S_un.S_addr = inet_addr(_host.c_str());

        if (server.sin_addr.S_un.S_addr == 0xffffffff) {
            hostent *hst;
            hst = gethostbyname(_host.c_str());
            if (!hst) {
                OnErrorOnSending();
                return;
            }

            unsigned int **addrptr;
            addrptr = (unsigned int **)hst->h_addr_list;

            while (*addrptr) {
                server.sin_addr.S_un.S_addr = **addrptr;
                if (!connect(sock, (struct sockaddr *)&server, sizeof(server))) {
                    break;
                }
                ++addrptr;
            }

            if (!*addrptr) {
                OnErrorOnSending();
                return;
            }
        }
        else {
            if (connect(sock, (struct sockaddr *)&server, sizeof(server))) {
                OnErrorOnSending();
                return;
            }
        }

        std::ostringstream req;
        char buf[4096];
        int n;

        if (_aborted) {
            goto onaborted;
        }

        req << "GET " << _path << " HTTP/1.0\r\n";
        for (header_container::const_iterator p = _requestHeaders.begin(); p != _requestHeaders.end(); ++p) {
            req << p->first << ": " << p->second << "\r\n";
        }
        req << "\r\n";

        n = send(sock, req.str().c_str(), req.str().length(), 0);
        SetReadyState(2);
        if (n < 0) {
            OnErrorOnSending();
            return;
        }

        SetReadyState(3);

        fd_set fds, readfds;
        FD_ZERO(&readfds);
        FD_SET(sock, &readfds);
        timeval tv;
        tv.tv_sec = 0;
        tv.tv_usec = 50 * 1000;
        while (!_aborted && n > 0) {
            memcpy(&fds, &readfds, sizeof(fd_set));
            while (select(0, &fds, NULL, NULL, &tv) == 0) {
                if (_aborted) {
                    goto onaborted;
                }
            }

            memset(buf, 0, sizeof(buf));
            n = recv(sock, buf, sizeof(buf), 0);
            _responseData.insert(_responseData.end(), buf, buf + n);
            if (n < 0) {
                OnErrorOnSending();
                return;
            }
        }

    onaborted:
        closesocket(sock);
        WSACleanup();

        if (_hThread) {
            CloseHandle(_hThread);
        }
        _hThread = NULL;

        if (!_aborted) {
            ParseResponse();
            SetReadyState(4);
        }
    }

    void OnReadyStateChange() {
        if (_async && _target->IsValid(TJS_IGNOREPROP, L"onreadystatechange", NULL, _target) == TJS_S_TRUE) {
            tTJSVariant val;
            if (_target->PropGet(TJS_IGNOREPROP, L"onreadystatechange", NULL, &val, _target) < 0) {
                ttstr msg = TJS_W("can't get member: onreadystatechange");
                TVPThrowExceptionMessage(msg.c_str());
            }

            iTJSDispatch2 *method = val.AsObject();
            method->FuncCall(0, NULL, NULL, NULL, 0, NULL, _target);
            method->Release();
        }
    }


    static unsigned __stdcall StartProc(void *arg)
    {
        ((NI_XMLHttpRequest*)arg)->_Send();
        return 0;
    }

    /*
     * �}�[�W�����ɒP�ɐV�����l�ŏ㏑������
     */
    void SetRequestHeader(const ttstr &header, const ttstr &value)
    {
        if (_readyState != 1) {
            TVPThrowExceptionMessage(TJS_W("INVALID_STATE_ERR"));
        }

        if (!IsValidHeaderName(header)) {
            TVPThrowExceptionMessage(TJS_W("SYNTAX_ERR"));
        }

        if (!IsValidHeaderValue(value)) {
            TVPThrowExceptionMessage(TJS_W("SYNTAX_ERR"));
        }

        std::string sheader;
        std::string svalue;
        std::copy(header.c_str(), header.c_str() + header.length(), std::back_inserter(sheader));
        std::copy(value.c_str(), value.c_str() + value.length(), std::back_inserter(svalue));

        _requestHeaders.erase(sheader);
        _requestHeaders.insert(std::pair<std::string, std::string>(sheader, svalue));
    }

    // for debug
    void PrintRequestHeaders(void)
    {
        for (header_container::const_iterator p = _requestHeaders.begin(); p != _requestHeaders.end(); ++p) {
            TVPAddLog((p->first + ": " + p->second).c_str());
        }
    }

    void Abort(void)
    {
        _aborted = true;

        if (_hThread) {
            WaitForSingleObject(_hThread, INFINITE);
        }

        Initialize();
    }

private:
    void OnErrorOnSending()
    {
        _readyState = 4;
    }

    void RaiseExceptionIfNotResponsed(void) const
    {
        if (_readyState != 3 && _readyState != 4) {
            TVPThrowExceptionMessage(TJS_W("INVALID_STATE_ERR"));
        }
    }

    bool IsValidUserInfo(const ttstr &username, const ttstr &password)
    {
        if (username.length() == 0 || password.length() == 0) {
            return true;
        }

        return std::find_if(username.c_str(), username.c_str() + username.length(), NI_XMLHttpRequest::IsInvalidUserInfoCharacter) ==
            username.c_str() + username.length() &&
            std::find_if(password.c_str(), password.c_str() + password.length(), NI_XMLHttpRequest::IsInvalidUserInfoCharacter) ==
            password.c_str() + password.length();
    }

    static bool IsInvalidUserInfoCharacter(tjs_char c)
    {
        return c > 127; // non US-ASCII character
    }

    bool IsValidHeaderName(const ttstr &header)
    {
        return header.length() > 0 &&
            std::find_if(header.c_str(), header.c_str() + header.length(), NI_XMLHttpRequest::IsInvalidHeaderNameCharacter) ==
            header.c_str() + header.length();
    }

    static bool IsInvalidHeaderNameCharacter(tjs_char c)
    {
        if (c > 127) return true; // non US-ASCII character
        if (c <= 31 || c == 127) return true; // CTL
        const std::string separators = "()<>@,;:\\\"/[]?={} \t";
        return std::find(separators.begin(), separators.end(), c) != separators.end();
    }

    bool IsValidHeaderValue(const ttstr &value)
    {
        // �P���̂��� "\r\n" �͋����Ȃ����Ƃɂ���
        if (wcsstr(value.c_str(), L"\r\n")) {
            return false;
        }

        return true;
    }

    void ParseResponse()
    {
        boost::reg_expression<char> re("\\AHTTP/[0-9]+\\.[0-9]+ ([0-9][0-9][0-9])",
            boost::regbase::normal|boost::regbase::use_except|boost::regbase::nocollate);
        boost::match_results<const char*> what;
        bool matched = boost::regex_search(_responseData.begin(), what, re, boost::match_default);

        if (matched) {
            _responseStatus = TJSStringToInteger(ttstr(what[1].first, what[1].second - what[1].first).c_str());
        }

        boost::reg_expression<char> re2("\r\n\r\n",
                                       boost::regbase::normal|boost::regbase::use_except|boost::regbase::nocollate);
        _responseBody.clear();
        if (boost::regex_search(_responseData.begin(), what, re2, boost::match_default)) {
            _responseBody.resize(_responseData.end() - what[0].second);
            std::copy(const_cast<char*>(what[0].second), _responseData.end(), _responseBody.begin());
        }
    }

    std::string EncodeBase64(const std::string target)
    {
        std::string result = "";
        std::vector<char> r;

        int len, restlen;
        len = restlen = target.length();

        while (restlen >= 3) {
            char t1 = target[len - restlen];
            char t2 = target[len - restlen + 1];
            char t3 = target[len - restlen + 2];

            r.push_back(t1 >> 2);
            r.push_back(((t1 & 3) << 4) | (t2 >> 4));
            r.push_back(((t2 & 0x0f) << 2) | (t3 >> 6));
            r.push_back(t3 & 0x3f);

            restlen -= 3;
        }

        if (restlen == 1) {
            char t1 = target[len - restlen];
            char t2 = '\0';
            r.push_back(t1 >> 2);
            r.push_back(((t1 & 3) << 4) | (t2 >> 4));

        }
        else if (restlen == 2) {
            char t1 = target[len - restlen];
            char t2 = target[len - restlen + 1];
            char t3 = '\0';
            r.push_back(t1 >> 2);
            r.push_back(((t1 & 3) << 4) | (t2 >> 4));
            r.push_back(((t2 & 0x0f) << 2) | (t3 >> 6));
        }

        for (std::vector<char>::const_iterator p = r.begin(); p != r.end(); ++p) {
            result.append(1, "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/"[*p]);
        }

        if (restlen > 0) {
            result.append(3 - restlen, '=');
        }

        return result;
    }
private:
    tjs_int _readyState;
    ttstr method;
    bool _async;
    int _port;
    std::string _host;
    std::string _path;
    std::vector<char> _responseData;
    std::vector<char> _responseBody;
    int _responseStatus;

    typedef std::map<std::string, std::string> header_container;
    header_container _requestHeaders;
    HANDLE _hThread;
    bool _aborted;
    iTJSDispatch2 *_target;
};
//---------------------------------------------------------------------------
/*
    ����� NI_XMLHttpRequest �̃I�u�W�F�N�g���쐬���ĕԂ������̊֐��ł��B
    ��q�� TJSCreateNativeClassForPlugin �̈����Ƃ��ēn���܂��B
*/
static iTJSNativeInstance * TJS_INTF_METHOD Create_NI_XMLHttpRequest()
{
    return new NI_XMLHttpRequest();
}
//---------------------------------------------------------------------------
/*
    TJS2 �̃l�C�e�B�u�N���X�͈�ӂ� ID �ŋ�ʂ���Ă���K�v������܂��B
    ����͌�q�� TJS_BEGIN_NATIVE_MEMBERS �}�N���Ŏ����I�Ɏ擾����܂����A
    ���� ID ���i�[����ϐ����ƁA���̕ϐ��������Ő錾���܂��B
    �����l�ɂ͖����� ID ��\�� -1 ���w�肵�Ă��������B
*/
#define TJS_NATIVE_CLASSID_NAME ClassID_XMLHttpRequest
static tjs_int32 TJS_NATIVE_CLASSID_NAME = -1;
//---------------------------------------------------------------------------
/*
    TJS2 �p�́u�N���X�v���쐬���ĕԂ��֐��ł��B
*/
static iTJSDispatch2 * Create_NC_XMLHttpRequest()
{
    /*
        �܂��A�N���X�̃x�[�X�ƂȂ�N���X�I�u�W�F�N�g���쐬���܂��B
        ����ɂ� TJSCreateNativeClassForPlugin ��p���܂��B
        TJSCreateNativeClassForPlugin �̑�P�����̓N���X���A��Q������
        �l�C�e�B�u�C���X�^���X��Ԃ��֐����w�肵�܂��B
        �쐬�����I�u�W�F�N�g���ꎞ�I�Ɋi�[���郍�[�J���ϐ��̖��O��
        classobj �ł���K�v������܂��B
    */
    tTJSNativeClassForPlugin * classobj =
        TJSCreateNativeClassForPlugin(TJS_W("XMLHttpRequest"), Create_NI_XMLHttpRequest);


    /*
        TJS_BEGIN_NATIVE_MEMBERS �}�N���ł��B�����ɂ� TJS2 ���Ŏg�p����N���X��
        ���w�肵�܂��B
        ���̃}�N���� TJS_END_NATIVE_MEMBERS �}�N���ŋ��܂ꂽ�ꏊ�ɁA�N���X��
        �����o�ƂȂ�ׂ����\�b�h��v���p�e�B�̋L�q�����܂��B
    */
    TJS_BEGIN_NATIVE_MEMBERS(/*TJS class name*/XMLHttpRequest)

        TJS_DECL_EMPTY_FINALIZE_METHOD


        TJS_BEGIN_NATIVE_CONSTRUCTOR_DECL(
            /*var.name*/_this,
            /*var.type*/NI_XMLHttpRequest,
            /*TJS class name*/XMLHttpRequest)
        {
            // NI_XMLHttpRequest::Construct �ɂ����e���L�q�ł���̂�
            // �����ł͉������Ȃ�
            return TJS_S_OK;
        }
        TJS_END_NATIVE_CONSTRUCTOR_DECL(/*TJS class name*/XMLHttpRequest)


        TJS_BEGIN_NATIVE_METHOD_DECL(/*func. name*/open)
        {
            TJS_GET_NATIVE_INSTANCE(/*var. name*/_this,
                /*var. type*/NI_XMLHttpRequest);

            if (numparams < 2) return TJS_E_BADPARAMCOUNT;

            if (param[0]->Type() != tvtString || param[1]->Type() != tvtString) {
                return TJS_E_INVALIDPARAM;
            }

            bool async;
            if (numparams < 3) {
                async = true;
            }
            else {
                async = (bool)*param[2];
            }

            ttstr username;
            if (numparams < 4) {
                username = "";
            }
            else {
                if (param[3]->Type() == tvtString) {
                    username = *param[3];
                }
                else {
                    return TJS_E_INVALIDPARAM;
                }
            }

            ttstr password;
            if (numparams < 5) {
                password = "";
            }
            else {
                if (param[4]->Type() == tvtString) {
                    password = *param[4];
                }
                else {
                    return TJS_E_INVALIDPARAM;
                }
            }

            _this->Open(ttstr(*param[0]), ttstr(*param[1]), async, username, password);

            return TJS_S_OK;
        }
        TJS_END_NATIVE_METHOD_DECL(/*func. name*/open)

        TJS_BEGIN_NATIVE_METHOD_DECL(/*func. name*/send)
        {
            TJS_GET_NATIVE_INSTANCE(/*var. name*/_this,
                /*var. type*/NI_XMLHttpRequest);

            _this->Send();
            if (result) {
                result->Clear();
            }

            return TJS_S_OK;
        }
        TJS_END_NATIVE_METHOD_DECL(/*func. name*/send)


        TJS_BEGIN_NATIVE_METHOD_DECL(/*func. name*/setRequestHeader)
        {
            TJS_GET_NATIVE_INSTANCE(/*var. name*/_this,
                /*var. type*/NI_XMLHttpRequest);

            if (numparams < 2) return TJS_E_BADPARAMCOUNT;
            if (param[0]->Type() != tvtString || param[1]->Type() != tvtString) {
                return TJS_E_INVALIDPARAM;
            }

            _this->SetRequestHeader(ttstr(*param[0]), ttstr(*param[1]));
            if (result) {
                result->Clear();
            }

            return TJS_S_OK;
        }
        TJS_END_NATIVE_METHOD_DECL(/*func. name*/setRequestHeader)


        TJS_BEGIN_NATIVE_METHOD_DECL(/*func. name*/printRequestHeaders)
        {
            TJS_GET_NATIVE_INSTANCE(/*var. name*/_this,
                /*var. type*/NI_XMLHttpRequest);

            _this->PrintRequestHeaders();
            if (result) {
                result->Clear();
            }

            return TJS_S_OK;
        }
        TJS_END_NATIVE_METHOD_DECL(/*func. name*/printRequestHeaders)


        TJS_BEGIN_NATIVE_METHOD_DECL(/*func. name*/abort)
        {
            TJS_GET_NATIVE_INSTANCE(/*var. name*/_this,
                /*var. type*/NI_XMLHttpRequest);

            _this->Abort();
            if (result) {
                result->Clear();
            }

            return TJS_S_OK;
        }
        TJS_END_NATIVE_METHOD_DECL(/*func. name*/abort)


        TJS_BEGIN_NATIVE_PROP_DECL(readyState)
        {
            TJS_BEGIN_NATIVE_PROP_GETTER
            {
                TJS_GET_NATIVE_INSTANCE(/*var. name*/_this,
                    /*var. type*/NI_XMLHttpRequest);

                if (result) {
                    *result = (tTVInteger)_this->GetReadyState();
                }

                return TJS_S_OK;
            }
            TJS_END_NATIVE_PROP_GETTER

            TJS_DENY_NATIVE_PROP_SETTER
        }
        TJS_END_NATIVE_PROP_DECL(readyState)


        TJS_BEGIN_NATIVE_PROP_DECL(responseText)
        {
            TJS_BEGIN_NATIVE_PROP_GETTER
            {
                TJS_GET_NATIVE_INSTANCE(/*var. name*/_this,
                    /*var. type*/NI_XMLHttpRequest);

                if (result) {
                    const std::vector<char>* data = _this->GetResponseText();
                    tjs_uint8 *d = new tjs_uint8[data->size()];
                    std::copy(data->begin(), data->end(), d);
                    *result = TJSAllocVariantOctet(d, data->size());
                    delete[] d;
                }

                return TJS_S_OK;
            }
            TJS_END_NATIVE_PROP_GETTER

            TJS_DENY_NATIVE_PROP_SETTER
        }
        TJS_END_NATIVE_PROP_DECL(responseText)



        TJS_BEGIN_NATIVE_PROP_DECL(status)
        {
            TJS_BEGIN_NATIVE_PROP_GETTER
            {
                TJS_GET_NATIVE_INSTANCE(/*var. name*/_this,
                    /*var. type*/NI_XMLHttpRequest);

                if (result) {
                    *result = _this->GetResponseStatus();
                }

                return TJS_S_OK;
            }
            TJS_END_NATIVE_PROP_GETTER

            TJS_DENY_NATIVE_PROP_SETTER
        }
        TJS_END_NATIVE_PROP_DECL(status)

        TJS_BEGIN_NATIVE_PROP_DECL(statusText)
        {
            TJS_BEGIN_NATIVE_PROP_GETTER
            {
                TJS_GET_NATIVE_INSTANCE(/*var. name*/_this,
                    /*var. type*/NI_XMLHttpRequest);

                if (result) {
                    *result = _this->GetResponseStatusText();
                }

                return TJS_S_OK;
            }
            TJS_END_NATIVE_PROP_GETTER

            TJS_DENY_NATIVE_PROP_SETTER
        }
        TJS_END_NATIVE_PROP_DECL(statusText)

    TJS_END_NATIVE_MEMBERS

    /*
        ���̊֐��� classobj ��Ԃ��܂��B
    */
    return classobj;
}
//---------------------------------------------------------------------------
/*
    TJS_NATIVE_CLASSID_NAME �͈ꉞ undef ���Ă������ق����悢�ł��傤
*/
#undef TJS_NATIVE_CLASSID_NAME
//---------------------------------------------------------------------------






//---------------------------------------------------------------------------
#pragma argsused
int WINAPI DllEntryPoint(HINSTANCE hinst, unsigned long reason,
    void* lpReserved)
{
    return 1;
}
//---------------------------------------------------------------------------
static tjs_int GlobalRefCountAtInit = 0;
extern "C" HRESULT _stdcall _export V2Link(iTVPFunctionExporter *exporter)
{
    // �X�^�u�̏�����(�K���L�q����)
    TVPInitImportStub(exporter);

    tTJSVariant val;

    // TJS �̃O���[�o���I�u�W�F�N�g���擾����
    iTJSDispatch2 * global = TVPGetScriptDispatch();


    //-----------------------------------------------------------------------
    // 1 �܂��N���X�I�u�W�F�N�g���쐬
    iTJSDispatch2 * tjsclass = Create_NC_XMLHttpRequest();

    // 2 tjsclass �� tTJSVariant �^�ɕϊ�
    val = tTJSVariant(tjsclass);

    // 3 ���ł� val �� tjsclass ��ێ����Ă���̂ŁAtjsclass ��
    //   Release ����
    tjsclass->Release();


    // 4 global �� PropSet ���\�b�h��p���A�I�u�W�F�N�g��o�^����
    global->PropSet(
        TJS_MEMBERENSURE, // �����o���Ȃ������ꍇ�ɂ͍쐬����悤�ɂ���t���O
        TJS_W("XMLHttpRequest"), // �����o�� ( ���Ȃ炸 TJS_W( ) �ň͂� )
        NULL, // �q���g ( �{���̓����o���̃n�b�V���l�����ANULL �ł��悢 )
        &val, // �o�^����l
        global // �R���e�L�X�g ( global �ł悢 )
        );
    //-----------------------------------------------------------------------


    // - global �� Release ����
    global->Release();

    // �����A�o�^����֐�����������ꍇ�� 1 �` 4 ���J��Ԃ�


    // val ���N���A����B
    // ����͕K���s���B�������Ȃ��� val ���ێ����Ă���I�u�W�F�N�g
    // �� Release ���ꂸ�A���Ɏg�� TVPPluginGlobalRefCount �����m�ɂȂ�Ȃ��B
    val.Clear();


    // ���̎��_�ł� TVPPluginGlobalRefCount �̒l��
    GlobalRefCountAtInit = TVPPluginGlobalRefCount;
    // �Ƃ��čT���Ă����BTVPPluginGlobalRefCount �͂��̃v���O�C������
    // �Ǘ�����Ă��� tTJSDispatch �h���I�u�W�F�N�g�̎Q�ƃJ�E���^�̑��v�ŁA
    // ������ɂ͂���Ɠ������A����������Ȃ��Ȃ��ĂȂ��ƂȂ�Ȃ��B
    // �����Ȃ��ĂȂ���΁A�ǂ����ʂ̂Ƃ���Ŋ֐��Ȃǂ��Q�Ƃ���Ă��āA
    // �v���O�C���͉���ł��Ȃ��ƌ������ƂɂȂ�B

    return S_OK;
}
//---------------------------------------------------------------------------
extern "C" HRESULT _stdcall _export V2Unlink()
{
    // �g���g��������A�v���O�C����������悤�Ƃ���Ƃ��ɌĂ΂��֐��B

    // �������炩�̏����Ńv���O�C��������ł��Ȃ��ꍇ��
    // ���̎��_�� E_FAIL ��Ԃ��悤�ɂ���B
    // �����ł́ATVPPluginGlobalRefCount �� GlobalRefCountAtInit ����
    // �傫���Ȃ��Ă���Ύ��s�Ƃ������Ƃɂ���B
    if(TVPPluginGlobalRefCount > GlobalRefCountAtInit) return E_FAIL;
        // E_FAIL ���A��ƁAPlugins.unlink ���\�b�h�͋U��Ԃ�


    /*
        �������A�N���X�̏ꍇ�A�����Ɂu�I�u�W�F�N�g���g�p���ł���v�Ƃ������Ƃ�
        �m�邷�ׂ�����܂���B��{�I�ɂ́APlugins.unlink �ɂ��v���O�C���̉����
        �댯�ł���ƍl���Ă������� (�������� Plugins.link �Ń����N������A�Ō��
        �Ńv���O�C������������A�v���O�����I���Ɠ����Ɏ����I�ɉ��������̂��g)�B
    */

    // TJS �̃O���[�o���I�u�W�F�N�g�ɓo�^���� XMLHttpRequest �N���X�Ȃǂ��폜����

    // - �܂��ATJS �̃O���[�o���I�u�W�F�N�g���擾����
    iTJSDispatch2 * global = TVPGetScriptDispatch();

    // - global �� DeleteMember ���\�b�h��p���A�I�u�W�F�N�g���폜����
    if(global)
    {
        // TJS ���̂����ɉ������Ă����Ƃ��Ȃǂ�
        // global �� NULL �ɂȂ蓾��̂� global �� NULL �łȂ�
        // ���Ƃ��`�F�b�N����

        global->DeleteMember(
            0, // �t���O ( 0 �ł悢 )
            TJS_W("XMLHttpRequest"), // �����o��
            NULL, // �q���g
            global // �R���e�L�X�g
            );
    }

    // - global �� Release ����
    if(global) global->Release();

    // �X�^�u�̎g�p�I��(�K���L�q����)
    TVPUninitImportStub();

    return S_OK;
}
//---------------------------------------------------------------------------
