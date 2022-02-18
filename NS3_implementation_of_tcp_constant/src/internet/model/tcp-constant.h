#ifndef TCPCONSTANT_H
#define TCPCONSTANT_H

#include "tcp-congestion-ops.h"
#include "ns3/tcp-recovery-ops.h"
#include "ns3/sequence-number.h"
#include "ns3/traced-value.h"
#include "ns3/event-id.h"
#include "ns3/tcp-socket-state.h"

namespace ns3 {

class Packet;
class TcpHeader;
class Time;
class EventId;


class TcpConstant : public TcpNewReno
{
public:
  void tunables_init();
  TcpConstant (void);
  TcpConstant (const TcpConstant& sock);
  virtual ~TcpConstant (void);

  static TypeId GetTypeId (void);
  std::string GetName () const;

  virtual void IncreaseWindow (Ptr<TcpSocketState> tcb, uint32_t segmentsAcked);
  virtual uint32_t GetSsThresh (Ptr<const TcpSocketState> tcb, uint32_t bytesInFlight);
  virtual Ptr<TcpCongestionOps> Fork ();  

//===================WESTWOOD=====================================================
  enum ProtocolType 
  {
    WESTWOOD,
    WESTWOODPLUS
  };

  enum FilterType 
  {
    NONE,
    TUSTIN
  };

  virtual void PktsAcked (Ptr<TcpSocketState> tcb, uint32_t packetsAcked,
                          const Time& rtt);

private:

  uint32_t m_cWndCnt {0}; //!< Linear increase counter

  ///==========WW==============
  /**
   * Update the total number of acknowledged packets during the current RTT
   *
   * \param [in] acked the number of packets the currently received ACK acknowledges
   */
  void UpdateAckedSegments (int acked);

  /**
   * Estimate the network's bandwidth
   *
   * \param [in] rtt the RTT estimation.
   * \param [in] tcb the socket state.
   */
  void EstimateBW (const Time& rtt, Ptr<TcpSocketState> tcb);

protected:
  /**
   * Slow start phase handler
   * \param tcb Transmission Control Block of the connection
   * \param segmentsAcked count of segments acked
   * \return Number of segments acked minus the difference between the receiver and sender Cwnd
   */
  virtual uint32_t SlowStart (Ptr<TcpSocketState> tcb, uint32_t segmentsAcked);
  /**
   * Congestion avoidance phase handler
   * \param tcb Transmission Control Block of the connection
   * \param segmentsAcked count of segments acked
   */
  virtual void CongestionAvoidance (Ptr<TcpSocketState> tcb, uint32_t segmentsAcked);


  //===================================WW==================================================
  TracedValue<double>    m_currentBW;              //!< Current value of the estimated BW
  double                 m_lastSampleBW;           //!< Last bandwidth sample
  double                 m_lastBW;                 //!< Last bandwidth sample after being filtered
  enum ProtocolType      m_pType;                  //!< 0 for Westwood, 1 for Westwood+
  enum FilterType        m_fType;                  //!< 0 for none, 1 for Tustin

  uint32_t               m_ackedSegments;          //!< The number of segments ACKed between RTTs
  bool                   m_IsCount;                //!< Start keeping track of m_ackedSegments for Westwood+ if TRUE
  EventId                m_bwEstimateEvent;        //!< The BW estimation event for Westwood+
  Time                   m_lastAck;                //!< The last ACK time


  //=================TCP CONSTANT======================
  
  uint32_t k_rounds;
  double beta;
  TracedValue<Time> prev_rtt {Seconds(0.0)}; //rtt_archive  
};

} // namespace ns3

#endif // TCPCONSTANT_H
