
import { Header } from "@/components/layout/Header";
import { Sidebar } from "@/components/layout/Sidebar";
import { Calendar } from "@/components/ui/calendar";
import { Card, CardContent, CardHeader, CardTitle } from "@/components/ui/card";
import { Badge } from "@/components/ui/badge";
import { FileText, Mail } from "lucide-react";
import { useState } from "react";
import type { CalendarEvent } from "@/types/calendar";

// Mock calendar events data
const mockEvents: CalendarEvent[] = [
  {
    id: "1",
    title: "Smith v. Jones Hearing",
    date: new Date("2025-04-20"),
    type: "court",
    case: "Smith v. Jones",
    associatedEmails: [
      {
        id: "e1",
        subject: "Hearing Schedule Update - Case #45921",
        sender: "Court Notifications",
        date: "Apr 15, 2025",
        preview: "Notice: The hearing has been scheduled for April 20th at 10:00 AM..."
      },
      {
        id: "e2",
        subject: "Pre-hearing Documentation",
        sender: "Robert Johnson",
        date: "Apr 16, 2025",
        preview: "I've attached all the required documents for next week's hearing..."
      }
    ]
  },
  {
    id: "2",
    title: "Client Meeting - Brighton Estates",
    date: new Date("2025-04-18"),
    type: "client",
    case: "Brighton Estates",
    associatedEmails: [
      {
        id: "e3",
        subject: "Meeting Confirmation",
        sender: "Maria Garcia",
        date: "Apr 14, 2025",
        preview: "This email confirms our meeting scheduled for April 18th..."
      }
    ]
  }
];

const CalendarPage = () => {
  const [date, setDate] = useState<Date | undefined>(new Date());
  const [selectedEvents, setSelectedEvents] = useState<CalendarEvent[]>([]);

  const handleDateSelect = (newDate: Date | undefined) => {
    setDate(newDate);
    if (newDate) {
      const events = mockEvents.filter(
        event => event.date.toDateString() === newDate.toDateString()
      );
      setSelectedEvents(events);
    } else {
      setSelectedEvents([]);
    }
  };

  return (
    <div className="flex h-screen bg-gray-50">
      <Sidebar />
      
      <div className="flex-1 flex flex-col overflow-hidden">
        <Header />
        
        <main className="flex-1 overflow-auto p-6">
          <div className="max-w-7xl mx-auto space-y-6">
            <div className="grid grid-cols-1 md:grid-cols-2 gap-6">
              <Card>
                <CardHeader>
                  <CardTitle>Court Calendar</CardTitle>
                </CardHeader>
                <CardContent>
                  <Calendar
                    mode="single"
                    selected={date}
                    onSelect={handleDateSelect}
                    className="rounded-md border shadow-sm"
                  />
                </CardContent>
              </Card>

              <Card>
                <CardHeader>
                  <CardTitle>Events & Associated Emails</CardTitle>
                </CardHeader>
                <CardContent>
                  {selectedEvents.length === 0 ? (
                    <p className="text-muted-foreground text-sm">
                      Select a date to view associated events and emails
                    </p>
                  ) : (
                    <div className="space-y-6">
                      {selectedEvents.map(event => (
                        <div key={event.id} className="space-y-4">
                          <div className="flex items-center gap-2">
                            <FileText className="h-5 w-5 text-legal-navy" />
                            <h3 className="font-medium">{event.title}</h3>
                            <Badge variant={event.type === 'court' ? 'destructive' : 'default'}>
                              {event.type}
                            </Badge>
                          </div>
                          
                          {event.case && (
                            <Badge variant="outline" className="mb-2">
                              {event.case}
                            </Badge>
                          )}
                          
                          <div className="pl-7 space-y-3">
                            <h4 className="text-sm font-medium flex items-center gap-2">
                              <Mail className="h-4 w-4" />
                              Associated Emails
                            </h4>
                            {event.associatedEmails.map(email => (
                              <div 
                                key={email.id}
                                className="border rounded-md p-3 hover:bg-muted/50 cursor-pointer transition-colors"
                              >
                                <div className="flex justify-between">
                                  <span className="font-medium">{email.sender}</span>
                                  <span className="text-sm text-muted-foreground">{email.date}</span>
                                </div>
                                <p className="text-sm font-medium">{email.subject}</p>
                                <p className="text-sm text-muted-foreground truncate">
                                  {email.preview}
                                </p>
                              </div>
                            ))}
                          </div>
                        </div>
                      ))}
                    </div>
                  )}
                </CardContent>
              </Card>
            </div>
          </div>
        </main>
      </div>
    </div>
  );
};

export default CalendarPage;
