#include "tcp-constant.h"
#include "ns3/log.h"
#include "ns3/simulator.h"
#include "rtt-estimator.h"
#include "tcp-socket-base.h"

NS_LOG_COMPONENT_DEFINE ("TcpConstant");

namespace ns3 {

NS_OBJECT_ENSURE_REGISTERED (TcpConstant);

//======================Public Functions======================================
void TcpConstant::tunables_init()
{
  beta = 0.015;
  k_rounds = 20;
}

TcpConstant::TcpConstant (void) :
  TcpNewReno (),
  m_currentBW (0),
  m_lastSampleBW (0),
  m_lastBW (0),
  m_ackedSegments (0),
  m_IsCount (false),
  m_lastAck (0)
{
  NS_LOG_FUNCTION (this);
  tunables_init();
}

TcpConstant::TcpConstant (const TcpConstant& sock) :
  TcpNewReno (sock),
  m_currentBW (sock.m_currentBW),
  m_lastSampleBW (sock.m_lastSampleBW),
  m_lastBW (sock.m_lastBW),
  m_pType (sock.m_pType),
  m_fType (sock.m_fType),
  m_IsCount (sock.m_IsCount),
  k_rounds (sock.k_rounds),
  beta (sock.beta)
{
  NS_LOG_FUNCTION (this);
  NS_LOG_LOGIC ("Invoked the copy constructor");
}

TcpConstant::~TcpConstant (void)
{
}

TypeId
TcpConstant::GetTypeId (void)
{
  static TypeId tid = TypeId("ns3::TcpConstant")
    .SetParent<TcpNewReno>()
    .SetGroupName ("Internet")
    .AddConstructor<TcpConstant>()
    .AddAttribute("FilterType", "Use this to choose no filter or Tustin's approximation filter",
                  EnumValue(TcpConstant::TUSTIN), MakeEnumAccessor(&TcpConstant::m_fType),
                  MakeEnumChecker(TcpConstant::NONE, "None", TcpConstant::TUSTIN, "Tustin"))
    .AddAttribute("ProtocolType", "Use this to let the code run as Westwood or WestwoodPlus",
                  EnumValue(TcpConstant::WESTWOOD),
                  MakeEnumAccessor(&TcpConstant::m_pType),
                  MakeEnumChecker(TcpConstant::WESTWOOD, "Westwood",TcpConstant::WESTWOODPLUS, "WestwoodPlus"))
    .AddTraceSource("EstimatedBW", "The estimated bandwidth",
                    MakeTraceSourceAccessor(&TcpConstant::m_currentBW),
                    "ns3::TracedValueCallback::Double")
  ;
  return tid;
}

//from Reno
std::string
TcpConstant::GetName () const
{
  return "TcpConstant";
}

//from Reno
void
TcpConstant::IncreaseWindow (Ptr<TcpSocketState> tcb, uint32_t segmentsAcked)
{
  NS_LOG_FUNCTION (this << tcb << segmentsAcked);

  // Linux tcp_in_slow_start() condition
  if (tcb->m_cWnd < tcb->m_ssThresh && k_rounds > 0)
    {
      NS_LOG_DEBUG ("In slow start, m_cWnd " << tcb->m_cWnd << " m_ssThresh " << tcb->m_ssThresh);
      segmentsAcked = SlowStart (tcb, segmentsAcked);

      //tcp-constant: store rtt_archive
      prev_rtt = tcb->m_lastRtt; 
      NS_LOG_UNCOND("rtt: " << prev_rtt << ", bw: " << m_currentBW << ", k: "<<k_rounds);
      k_rounds--;
    }
  else
    {
      NS_LOG_DEBUG ("In cong. avoidance, m_cWnd " << tcb->m_cWnd << " m_ssThresh " << tcb->m_ssThresh);
      CongestionAvoidance (tcb, segmentsAcked);
    }
}


//from reno
uint32_t
TcpConstant::GetSsThresh (Ptr<const TcpSocketState> tcb,
                          uint32_t bytesInFlight)
{
  NS_LOG_FUNCTION (this << tcb << bytesInFlight);

  // In Linux, it is written as:  return max(tp->snd_cwnd >> 1U, 2U);
  return std::max<uint32_t> (2 * tcb->m_segmentSize, tcb->m_cWnd / 2);
}


Ptr<TcpCongestionOps>
TcpConstant::Fork ()
{
  return CreateObject<TcpConstant> (*this);
}



//from WW
void
TcpConstant::PktsAcked (Ptr<TcpSocketState> tcb, uint32_t packetsAcked,
                        const Time& rtt)
{
  NS_LOG_FUNCTION (this << tcb << packetsAcked << rtt);

  if (rtt.IsZero ())
    {
      NS_LOG_WARN ("RTT measured is zero!");
      return;
    }

  m_ackedSegments += packetsAcked;

  if (m_pType == TcpConstant::WESTWOOD)
    {
      EstimateBW (rtt, tcb);
    }
  else if (m_pType == TcpConstant::WESTWOODPLUS)
    {
      if (!(rtt.IsZero () || m_IsCount))
        {
          m_IsCount = true;
          m_bwEstimateEvent.Cancel ();
          m_bwEstimateEvent = Simulator::Schedule (rtt, &TcpConstant::EstimateBW,
                                                   this, rtt, tcb);
        }
    }
}
//===================================END OF PUBLIC======================================






//==========================PRIVATE FUNCTIONS================================
//from WW
void
TcpConstant::EstimateBW (const Time &rtt, Ptr<TcpSocketState> tcb)
{
  NS_LOG_FUNCTION (this);

  NS_ASSERT (!rtt.IsZero ());

  m_currentBW = m_ackedSegments * tcb->m_segmentSize / rtt.GetSeconds ();

  if (m_pType == TcpConstant::WESTWOOD)
    {
      Time currentAck = Simulator::Now ();
      m_currentBW = m_ackedSegments * tcb->m_segmentSize / (currentAck - m_lastAck).GetSeconds ();
      m_lastAck = currentAck;
    }
  else if (m_pType == TcpConstant::WESTWOODPLUS)
    {
      m_currentBW = m_ackedSegments * tcb->m_segmentSize / rtt.GetSeconds ();
      m_IsCount = false;
    }

  m_ackedSegments = 0;
  NS_LOG_LOGIC ("Estimated BW: " << m_currentBW);

  // Filter the BW sample

  double alpha = 0.9;

  if (m_fType == TcpConstant::NONE)
    {
    }
  else if (m_fType == TcpConstant::TUSTIN)
    {
      double sample_bwe = m_currentBW;
      m_currentBW = (alpha * m_lastBW) + ((1 - alpha) * ((sample_bwe + m_lastSampleBW) / 2));
      m_lastSampleBW = sample_bwe;
      m_lastBW = m_currentBW;
    }

  NS_LOG_LOGIC ("Estimated BW after filtering: " << m_currentBW);
}

//==========================END OF PRIVATE================================












//=========================PROTECTED FUNCTIONS============================
//from Reno
uint32_t
TcpConstant::SlowStart (Ptr<TcpSocketState> tcb, uint32_t segmentsAcked)
{
  NS_LOG_FUNCTION (this << tcb << segmentsAcked);
  if (segmentsAcked >= 1)
    {
      uint32_t sndCwnd = tcb->m_cWnd;
      tcb->m_cWnd = std::min ((sndCwnd + (segmentsAcked * tcb->m_segmentSize)), (uint32_t)tcb->m_ssThresh);
      NS_LOG_INFO ("In SlowStart, updated to cwnd " << tcb->m_cWnd << " ssthresh " << tcb->m_ssThresh);
      return segmentsAcked - ((tcb->m_cWnd - sndCwnd) / tcb->m_segmentSize);
    }

  return 0;
}


//from Reno
void
TcpConstant::CongestionAvoidance (Ptr<TcpSocketState> tcb, uint32_t segmentsAcked)
{
  NS_LOG_FUNCTION (this << tcb << segmentsAcked);

  //tcp-constant: new rtt (rtt_var)
  TracedValue<Time> new_rtt = tcb->m_lastRtt;

  //tcp-constant: rtt difference ratio
  double prev_rtime = prev_rtt.Get().GetDouble();
  double new_rtime = new_rtt.Get().GetDouble();
  double del_rtime;
  if(prev_rtime > new_rtime) del_rtime = prev_rtime - new_rtime;
  else del_rtime = new_rtime - prev_rtime;
  del_rtime = del_rtime / prev_rtime;

  TracedValue<Time> cwnd_;
  if(del_rtime > beta){
    //NS_LOG_UNCOND("recalculating cwnd, del_rtt: " << del_rtime << ", prtt: " << prev_rtt << ", nrtt: " << new_rtt << ", beta: " <<beta);
    cwnd_ = (m_currentBW/100000.0) * tcb->m_minRtt / tcb->m_segmentSize;

    //NS_LOG_UNCOND("bw: " << m_currentBW << ", min_rtt: " << tcb->m_minRtt << ", seg: " << tcb->m_segmentSize);;

    Time temp = cwnd_.Get();
    tcb->m_cWnd = temp.GetInteger();
    if(tcb->m_cWnd < 1) tcb->m_cWnd = 1;
    //NS_LOG_UNCOND("cwnd_trace: " << cwnd_ << ", cwnd: " << temp.GetInteger());
    prev_rtt = new_rtt;
  }
}
//==========================END OF PROTECTED================================


} // namespace ns3
