
import { useState } from "react";
import { Button } from "@/components/ui/button";
import { Card } from "@/components/ui/card";
import { Avatar, AvatarFallback } from "@/components/ui/avatar";
import { Badge } from "@/components/ui/badge";
import { 
  ArrowLeft, 
  Star, 
  Trash2, 
  Archive, 
  Reply, 
  MoreHorizontal, 
  Paperclip, 
  ChevronDown,
  Forward
} from "lucide-react";
import {
  Select,
  SelectContent,
  SelectItem,
  SelectTrigger,
  SelectValue,
} from "@/components/ui/select";

export function EmailDetail() {
  const [isExpanded, setIsExpanded] = useState(true);
  
  // Mock email data
  const email = {
    id: "1",
    sender: "Robert Johnson",
    senderEmail: "robert@clientfirm.com",
    recipients: ["john.doe@lawfirm.com"],
    cc: ["maria@lawpartners.com"],
    subject: "Smith v. Jones Case Documents",
    body: `<p>Hello John,</p>
           <p>I've attached the latest filings for review. Please let me know if you need any additional information from our side.</p>
           <p>Key points to note:</p>
           <ul>
             <li>The opposition's motion to dismiss was rejected</li>
             <li>We have 14 days to respond to their interrogatories</li>
             <li>A scheduling conference is set for next month</li>
           </ul>
           <p>Looking forward to your thoughts on how we should proceed.</p>
           <p>Best regards,<br>Robert Johnson<br>Client Representative<br>ClientFirm Inc.</p>`,
    date: "April 18, 2025 10:25 AM",
    attachments: [
      { name: "Smith_Opposition_Response.pdf", size: "1.4 MB" },
      { name: "Court_Order_April15.pdf", size: "650 KB" },
      { name: "Client_Instructions.docx", size: "230 KB" }
    ],
    category: "client",
    case: "Smith v. Jones",
    starred: true
  };
  
  return (
    <Card className="h-full flex flex-col overflow-hidden">
      {/* Email header */}
      <div className="p-4 border-b flex items-center justify-between">
        <div className="flex items-center gap-2">
          <Button variant="ghost" size="icon">
            <ArrowLeft className="h-5 w-5" />
          </Button>
          <h2 className="text-xl font-semibold">{email.subject}</h2>
        </div>
        
        <div className="flex items-center gap-1">
          <Button variant="ghost" size="icon">
            <Archive className="h-5 w-5" />
          </Button>
          <Button variant="ghost" size="icon">
            <Trash2 className="h-5 w-5" />
          </Button>
          <Button variant="ghost" size="icon">
            <Star className={`h-5 w-5 ${email.starred ? 'fill-yellow-400 text-yellow-400' : ''}`} />
          </Button>
          <Button variant="ghost" size="icon">
            <MoreHorizontal className="h-5 w-5" />
          </Button>
        </div>
      </div>
      
      {/* Case association */}
      <div className="px-4 py-3 border-b bg-muted/30 flex justify-between items-center">
        <div className="flex items-center gap-2">
          <span className="text-sm font-medium">Case:</span>
          <Badge variant="outline" className="bg-white">
            {email.case}
          </Badge>
        </div>
        
        <Select defaultValue={email.case}>
          <SelectTrigger className="w-[180px] h-8">
            <SelectValue placeholder="Associate with case" />
          </SelectTrigger>
          <SelectContent>
            <SelectItem value="Smith v. Jones">Smith v. Jones</SelectItem>
            <SelectItem value="Brighton Estates">Brighton Estates</SelectItem>
            <SelectItem value="ClientCorp Merger">ClientCorp Merger</SelectItem>
            <SelectItem value="none">No Case</SelectItem>
          </SelectContent>
        </Select>
      </div>
      
      {/* Email content */}
      <div className="flex-1 overflow-auto">
        <div className="p-4">
          <div className="flex items-start gap-3">
            <Avatar className="h-10 w-10">
              <AvatarFallback className="bg-legal-blue text-white">RJ</AvatarFallback>
            </Avatar>
            
            <div className="flex-1">
              <div className="flex justify-between items-start">
                <div>
                  <div className="font-medium">{email.sender}</div>
                  <div className="text-sm text-muted-foreground">{email.senderEmail}</div>
                </div>
                <div className="text-sm text-muted-foreground">{email.date}</div>
              </div>
              
              <div className="mt-2 text-sm">
                <span className="text-muted-foreground">To: </span>
                <span>john.doe@lawfirm.com</span>
              </div>
              
              {email.cc.length > 0 && (
                <div className="mt-1 text-sm">
                  <span className="text-muted-foreground">Cc: </span>
                  <span>{email.cc.join(", ")}</span>
                </div>
              )}
              
              {isExpanded ? (
                <div className="mt-6">
                  <div className="prose prose-sm max-w-none" dangerouslySetInnerHTML={{ __html: email.body }}></div>
                  
                  {email.attachments.length > 0 && (
                    <div className="mt-6 border-t pt-4">
                      <h4 className="text-sm font-medium mb-2 flex items-center gap-2">
                        <Paperclip className="h-4 w-4" />
                        Attachments ({email.attachments.length})
                      </h4>
                      <div className="grid grid-cols-1 gap-2">
                        {email.attachments.map((attachment, index) => (
                          <div
                            key={index}
                            className="flex items-center gap-2 p-2 border rounded-md hover:bg-muted/50 cursor-pointer"
                          >
                            <Paperclip className="h-4 w-4 text-muted-foreground" />
                            <div className="flex-1 min-w-0">
                              <p className="text-sm font-medium truncate">{attachment.name}</p>
                              <p className="text-xs text-muted-foreground">{attachment.size}</p>
                            </div>
                          </div>
                        ))}
                      </div>
                    </div>
                  )}
                </div>
              ) : (
                <Button 
                  variant="ghost" 
                  className="mt-2 h-auto p-1 text-sm"
                  onClick={() => setIsExpanded(true)}
                >
                  <ChevronDown className="h-4 w-4 mr-1" />
                  Show more
                </Button>
              )}
            </div>
          </div>
        </div>
      </div>
      
      {/* Email actions */}
      <div className="p-4 border-t">
        <div className="flex gap-2">
          <Button variant="outline">
            <Reply className="h-4 w-4 mr-2" />
            Reply
          </Button>
          <Button variant="outline">
            <Forward className="h-4 w-4 mr-2" />
            Forward
          </Button>
        </div>
      </div>
    </Card>
  );
}
