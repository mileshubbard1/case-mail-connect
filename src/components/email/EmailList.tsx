import { useState } from "react";
import { Button } from "@/components/ui/button";
import { Input } from "@/components/ui/input";
import { 
  Inbox, 
  Star, 
  AlertCircle, 
  Trash2, 
  MoreVertical,
  Search,
  ChevronDown,
  RefreshCw,
  ArrowUp,
  Mail
} from "lucide-react";
import { Checkbox } from "@/components/ui/checkbox";
import { Badge } from "@/components/ui/badge";

const mockEmails = [
  {
    id: "1",
    sender: "Robert Johnson",
    email: "robert@clientfirm.com",
    subject: "Re: Smith v. Jones Case Documents",
    preview: "I've attached the latest filings for review. Please let me know if...",
    date: "10:25 AM",
    read: false,
    starred: true,
    important: true,
    category: "client",
    case: "Smith v. Jones",
    priority: "high" as const
  },
  {
    id: "2",
    sender: "Court Notifications",
    email: "notifications@courts.gov",
    subject: "Hearing Schedule Update - Case #45921",
    preview: "Notice: The hearing originally scheduled for June 18th has been...",
    date: "Yesterday",
    read: true,
    starred: false,
    important: true,
    category: "court",
    case: "Brighton Estates",
    priority: "high" as const
  },
  {
    id: "3",
    sender: "Maria Garcia",
    email: "maria@lawpartners.com",
    subject: "Deposition Transcripts Ready",
    preview: "The transcripts from yesterday's deposition are ready for your review...",
    date: "Apr 16",
    read: true,
    starred: false,
    important: false,
    category: "internal",
    case: "Smith v. Jones",
    priority: "medium" as const
  },
  {
    id: "4",
    sender: "Thomas Williams",
    email: "thomas@clientcorp.com",
    subject: "Updated Contract Draft",
    preview: "Please find attached the revised contract with the changes we...",
    date: "Apr 15",
    read: true,
    starred: true,
    important: false,
    category: "client",
    case: "ClientCorp Merger",
    priority: "low" as const
  },
  {
    id: "5",
    sender: "Legal Research Team",
    email: "research@lawfirm.com",
    subject: "Research Results: Precedent Cases",
    preview: "As requested, here are the precedent cases that support our argument...",
    date: "Apr 14",
    read: true,
    starred: false,
    important: false,
    category: "internal",
    case: "Brighton Estates",
    priority: "low" as const
  },
];

interface EmailItemProps {
  email: typeof mockEmails[0];
  isSelected: boolean;
  onSelect: (id: string) => void;
  onClick: (id: string) => void;
}

const EmailItem = ({ email, isSelected, onSelect, onClick }: EmailItemProps) => {
  const getPriorityColor = (priority: string) => {
    switch (priority) {
      case 'high':
        return 'text-red-500';
      case 'medium':
        return 'text-yellow-500';
      case 'low':
        return 'text-blue-500';
      default:
        return 'text-gray-500';
    }
  };

  return (
    <div 
      className={`flex items-center gap-3 p-3 hover:bg-muted cursor-pointer transition-colors border-b ${!email.read ? 'bg-legal-light/30' : ''}`}
      onClick={() => onClick(email.id)}
    >
      <div className="flex items-center gap-2">
        <Checkbox 
          checked={isSelected} 
          onCheckedChange={() => onSelect(email.id)}
          onClick={(e) => e.stopPropagation()}
        />
        <Button 
          variant="ghost" 
          size="sm" 
          className="p-0 h-auto" 
          onClick={(e) => { 
            e.stopPropagation(); 
          }}
        >
          <Star className={`h-4 w-4 ${email.starred ? 'fill-yellow-400 text-yellow-400' : ''}`} />
        </Button>
        <div className="flex items-center">
          {email.priority === 'high' ? (
            <ArrowUp className={`h-4 w-4 ${getPriorityColor(email.priority)}`} />
          ) : (
            <Mail className={`h-4 w-4 ${getPriorityColor(email.priority || 'default')}`} />
          )}
        </div>
      </div>
      
      <div className="flex-1 min-w-0">
        <div className="flex items-center gap-2">
          <p className={`text-sm font-medium truncate ${!email.read ? 'font-semibold' : ''}`}>
            {email.sender}
          </p>
          
          <Badge 
            variant="outline" 
            className="text-xs py-0 h-5"
          >
            {email.case}
          </Badge>
        </div>
        <p className={`text-sm truncate ${!email.read ? 'font-medium' : ''}`}>
          {email.subject}
        </p>
        <p className="text-xs text-muted-foreground truncate">{email.preview}</p>
      </div>
      
      <div className="flex flex-col items-end gap-1">
        <span className="text-xs whitespace-nowrap text-muted-foreground">{email.date}</span>
        {email.category === "court" && (
          <Badge className="bg-red-100 text-red-800 hover:bg-red-200 text-[10px]">Court</Badge>
        )}
        {email.category === "client" && (
          <Badge className="bg-blue-100 text-blue-800 hover:bg-blue-200 text-[10px]">Client</Badge>
        )}
        {email.category === "internal" && (
          <Badge className="bg-gray-100 text-gray-800 hover:bg-gray-200 text-[10px]">Internal</Badge>
        )}
      </div>
    </div>
  );
};

export function EmailList() {
  const [selectedEmails, setSelectedEmails] = useState<string[]>([]);
  const [searchQuery, setSearchQuery] = useState("");
  const [priorityFilter, setPriorityFilter] = useState<string | null>(null);

  const handleSelectEmail = (id: string) => {
    if (selectedEmails.includes(id)) {
      setSelectedEmails(selectedEmails.filter(emailId => emailId !== id));
    } else {
      setSelectedEmails([...selectedEmails, id]);
    }
  };

  const handleSelectAll = () => {
    if (selectedEmails.length === mockEmails.length) {
      setSelectedEmails([]);
    } else {
      setSelectedEmails(mockEmails.map(email => email.id));
    }
  };

  const handleEmailClick = (id: string) => {
    console.log("Opening email:", id);
  };

  const sortedEmails = [...mockEmails].sort((a, b) => {
    const priorityOrder = { high: 0, medium: 1, low: 2, undefined: 3 };
    const priorityDiff = (priorityOrder[a.priority || 'undefined'] || 3) - 
                        (priorityOrder[b.priority || 'undefined'] || 3);
    
    if (priorityDiff !== 0) return priorityDiff;
    
    return new Date(b.date).getTime() - new Date(a.date).getTime();
  });

  const filteredEmails = priorityFilter
    ? sortedEmails.filter(email => email.priority === priorityFilter)
    : sortedEmails;

  return (
    <div className="flex flex-col h-full border rounded-md overflow-hidden">
      <div className="p-3 border-b flex gap-2">
        <div className="relative flex-1">
          <Search className="absolute left-2 top-1/2 transform -translate-y-1/2 h-4 w-4 text-muted-foreground" />
          <Input
            placeholder="Search emails..."
            className="pl-8"
            value={searchQuery}
            onChange={(e) => setSearchQuery(e.target.value)}
          />
        </div>
        <Button variant="outline" size="icon">
          <RefreshCw className="h-4 w-4" />
        </Button>
      </div>
      
      <div className="border-b p-2 flex items-center gap-1">
        <Checkbox 
          checked={selectedEmails.length === mockEmails.length && mockEmails.length > 0} 
          onCheckedChange={handleSelectAll}
          className="ml-1"
        />
        <Button variant="ghost" size="sm" className="p-1 h-8">
          <ChevronDown className="h-4 w-4" />
        </Button>
        
        <div className="flex items-center ml-2 gap-1">
          <Button 
            variant="ghost" 
            size="sm"
            disabled={selectedEmails.length === 0}
          >
            <Inbox className="h-4 w-4 mr-1" />
            Archive
          </Button>
          <Button 
            variant="ghost" 
            size="sm"
            disabled={selectedEmails.length === 0}
          >
            <Trash2 className="h-4 w-4 mr-1" />
            Delete
          </Button>
          <Button 
            variant="ghost" 
            size="sm"
            disabled={selectedEmails.length === 0}
          >
            <MoreVertical className="h-4 w-4" />
          </Button>
        </div>

        <div className="ml-auto flex gap-2">
          <Button
            variant={priorityFilter === 'high' ? 'secondary' : 'ghost'}
            size="sm"
            onClick={() => setPriorityFilter(priorityFilter === 'high' ? null : 'high')}
          >
            <ArrowUp className={`h-4 w-4 mr-1 text-red-500`} />
            High Priority
          </Button>
        </div>
      </div>
      
      <div className="flex-1 overflow-y-auto">
        {filteredEmails.map(email => (
          <EmailItem 
            key={email.id} 
            email={email} 
            isSelected={selectedEmails.includes(email.id)}
            onSelect={handleSelectEmail}
            onClick={handleEmailClick}
          />
        ))}
      </div>
    </div>
  );
}
